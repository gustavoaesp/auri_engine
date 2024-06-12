#include "stages/render_stage_geometry.hpp"
#include "scene/scene.hpp"

#include "transform/matrix.hpp"

namespace eng
{

RStageGeometry::RStageGeometry(IRenderBackend* backend_ref, uint32_t width, uint32_t height):
    backend_ref_(backend_ref)
{
    RTexture *albedo = backend_ref->CreateImage2D(
        width, height,
        EFormat::kFormat_R8G8B8A8_UNORM
    );
    RTexture *normals = backend_ref->CreateImage2D(
        width, height,
        EFormat::kFormat_R10G10B10A2
    );
    RTexture *depth = backend_ref->CreateImage2D(
        width, height,
        EFormat::kFormat_R24G8
    );

    std::array<RRenderPassAttachment, 2> rpass_attachments{
        RRenderPassAttachment{
            .texture = albedo,
            .load_op = RPassAttachmentLoadOp::kClear,
            .store_op = RPassAttachmentStoreOp::kStore
        },
        RRenderPassAttachment{
            .texture = normals,
            .load_op = RPassAttachmentLoadOp::kClear,
            .store_op = RPassAttachmentStoreOp::kStore
        }
    };

    RRenderPassAttachment rpass_depth_attachment{
        .texture = depth,
        .load_op = RPassAttachmentLoadOp::kClear,
        .store_op = RPassAttachmentStoreOp::kStore
    };

    render_pass_ = std::unique_ptr<RRenderPass>(
        backend_ref->CreateRenderPass(
            rpass_attachments.data(),
            rpass_attachments.size(),
            &rpass_depth_attachment
        )
    );

    g_buffer_ = std::unique_ptr<RFramebuffer>(
        backend_ref->CreateFramebuffer(
            render_pass_.get(),
            std::array<RTexture*, 2>{
                albedo, normals
            }.data(),
            2,
            depth
        )
    );

    std::array<RDescriptorLayoutBinding, 4> buffer_bindings;
    std::array<RDescriptorLayoutBinding, 8> texture_bindings;
    for (int i = 0; i < buffer_bindings.size(); ++i) {
        buffer_bindings[i].bindingIndex = i;
        buffer_bindings[i].bindingStageAccessFlags =
            RDescriptorLayoutBindingStageAccess::EShaderStageVertexBit;
        buffer_bindings[i].type = RDescriptorLayoutBindingType::kUniformBuffer;
    }
    for (int i = 0; i < texture_bindings.size(); ++i) {
        texture_bindings[i].bindingIndex = i;
        texture_bindings[i].bindingStageAccessFlags =
            RDescriptorLayoutBindingStageAccess::EShaderStageFragmentBit;
        texture_bindings[i].type = RDescriptorLayoutBindingType::kTextureSampler;
    }
    desc_layout_buffers_ = std::unique_ptr<RDescriptorLayout>(
        backend_ref->CreateDescriptorLayout(
            buffer_bindings.data(), buffer_bindings.size()
        )
    );
    desc_layout_textures_ = std::unique_ptr<RDescriptorLayout>(
        backend_ref->CreateDescriptorLayout(
            texture_bindings.data(), texture_bindings.size()
        )
    );
    desc_pool_buffers_ = std::unique_ptr<RDescriptorPool>(
        backend_ref->CreateDescriptorPool(
            0x4000,
            RDescriptorLayoutBindingType::kUniformBuffer
        )
    );
    desc_pool_textures_ = std::unique_ptr<RDescriptorPool>(
        backend_ref->CreateDescriptorPool(
            0x4000,
            RDescriptorLayoutBindingType::kTextureSampler
        )
    );

    cmd_pool_ = std::unique_ptr<RCommandPool>(
        backend_ref->CreateCommandPool()
    );
    cmd_buffer_ = std::unique_ptr<RCommandBuffer>(
        cmd_pool_->CreateCommandBuffer(true)
    );

    main_vert_ = std::unique_ptr<RShader>(
        backend_ref->CreateShader(
            "shaders/main.vert.spv",
            RShaderPipelineBind::kShaderVertex
        )
    );
    main_frag_ = std::unique_ptr<RShader>(
        backend_ref->CreateShader(
            "shaders/main.frag.spv",
            RShaderPipelineBind::kShaderFragment
        )
    );

    RBlendState blend_state{};
    blend_state.num_blend_attachments = 2;
    RDepthStencilState depth_state{};
    depth_state.depth_test_enable = true;
    depth_state.depth_write_enable = true;
    depth_state.max_depth = 1.0f;

    main_pipeline_ = std::unique_ptr<RPipeline>(
        backend_ref->CreatePipeline(
            render_pass_.get(),
            &blend_state,
            &depth_state,
            (const RShader**)std::array<RShader*, 2>{
                main_vert_.get(),
                main_frag_.get()
            }.data(),
            2,
            RVertexType::kVertexPos3Nor3Tex2,
            RVertexType::kNoFormat,
            std::array<RDescriptorLayout*, 2>{
                desc_layout_buffers_.get(),
                desc_layout_textures_.get()
            }.data(),
            2
        )
    );

    view_projection_uniform_ = std::unique_ptr<RBuffer>(
        backend_ref->CreateBuffer(
            RBufferUsage::kUniform,
            sizeof(mtx4f),
            nullptr
        )
    );

    main_sampler_ = std::unique_ptr<RSampler>(
        backend_ref->CreateSampler(
            RSamplerFilterMode::kFilterLinear,
            RSamplerAddressMode::kRepeat
        )
    );
}

RStageGeometry::~RStageGeometry()
{
}

void RStageGeometry::Render(const RScene &scene)
{
    uint32_t buffers_count = 0;
    uint32_t textures_count = 0;

    if (scene.active_camera) {
        mtx4f view_proj = CreateViewMatrix(
            scene.active_camera->position,
            scene.active_camera->look_pos,
            scene.active_camera->up
        );

        view_proj *= CreatePerspectiveProjectionMatrix(
            16.0f/9.0f, scene.active_camera->fovy, 0.1f, 1000.0f
        );
        backend_ref_->UpdateBuffer(
            view_projection_uniform_.get(),
            &view_proj, 0, sizeof(mtx4f)
        );
    }

    std::array<vec4f, 3> clear_colors{
        vec4f(0.0f, 0.0f, 0.0f, 1.0f),
        vec4f(0.0f, 0.0f, 0.0f, 1.0f),
        vec4f(1.0f, 0.0f, 0.0f, 1.0f)
    };

    cmd_buffer_->Reset();
    cmd_buffer_->BeginRecord();
    cmd_buffer_->CmdBeginRenderPass(
        render_pass_.get(),
        g_buffer_.get(),
        clear_colors.data(), clear_colors.size(),
        0x00, false
    );
    for (const auto& scene_mesh: scene.scene_meshes) {
        mtx4f transform = CreateScaleMatrix(
            scene_mesh->scale(0),
            scene_mesh->scale(1),
            scene_mesh->scale(2)
        );
        transform *= scene_mesh->rotation.toMatrix();
        transform *= CreateTranslateMatrix(scene_mesh->position);
        RDescriptorSet *descriptor_set_buffers = nullptr;
        if (buffers_count >= descriptor_sets_buffer_.size()) {
            descriptor_sets_buffer_.push_back(
                std::unique_ptr<RDescriptorSet>(
                    desc_pool_buffers_->AllocateSet(desc_layout_buffers_.get())
                )
            );
            descriptor_set_buffers = descriptor_sets_buffer_.back().get();
        } else {
            descriptor_set_buffers = descriptor_sets_buffer_[buffers_count].get();
        }
        buffers_count++;

        if (!scene_mesh->uniform_buffer_transform) {
            scene_mesh->uniform_buffer_transform = std::unique_ptr<RBuffer>(
                backend_ref_->CreateBuffer(RBufferUsage::kUniform,
                sizeof(mtx4f), &transform)
            );
        } else {
            backend_ref_->UpdateBuffer(
                scene_mesh->uniform_buffer_transform.get(),
                &transform,
                0, sizeof(mtx4f)
            );
        }

        std::array<RBufferBinding, 2> buffer_bindings = {
            RBufferBinding{
                .buffer = view_projection_uniform_.get(),
                .start_offset = 0,
                .size = sizeof(mtx4f)
            },
            RBufferBinding{
                .buffer = scene_mesh->uniform_buffer_transform.get(),
                .start_offset = 0,
                .size = sizeof(mtx4f)
            }
        };
        descriptor_set_buffers->BindBuffers(
            0, buffer_bindings.data(), buffer_bindings.size()
        );
        for (int i = 0; i < scene_mesh->mesh->GetSubmeshCount(); ++i) {
            RDescriptorSet *descriptor_set_textures = nullptr;
            RSubmesh *submesh = scene_mesh->mesh->GetSubmesh(i);
            if (textures_count >= descriptor_sets_texture_.size()) {
                descriptor_sets_texture_.push_back(
                    std::unique_ptr<RDescriptorSet>(
                        desc_pool_textures_->AllocateSet(desc_layout_textures_.get())
                    )
                );
                descriptor_set_textures = descriptor_sets_texture_.back().get();
            } else {
                descriptor_set_textures = descriptor_sets_texture_[textures_count].get();
            }
            textures_count++;

            RTextureSamplerBinding sampler_binding{
                .texture = submesh->material.diffuse.get(),
                .sampler = main_sampler_.get()
            };
            descriptor_set_textures->BindTextures(
                0, &sampler_binding, 1
            );
            cmd_buffer_->CmdBindPipeline(main_pipeline_.get());
            cmd_buffer_->CmdSetScissor(0,0, 1600, 900);
            cmd_buffer_->CmdSetViewport(0, 0, 1600, 900);
            cmd_buffer_->CmdBindDescriptorSets(
                main_pipeline_.get(),
                (const RDescriptorSet**)std::array<RDescriptorSet*, 2> {
                    descriptor_set_buffers,
                    descriptor_set_textures
                }.data(),
                2
            );
            cmd_buffer_->CmdBindVertexBuffer(
                submesh->vertex_buffer.get(),
                0, 0
            );
            cmd_buffer_->CmdBindIndexBuffer(
                submesh->index_buffer.get(),
                0
            );
            cmd_buffer_->CmdDrawIndexed(
                submesh->indices_count,
                0, 0
            );
        }
    }
    cmd_buffer_->CmdEndRenderPass();
    cmd_buffer_->EndRecord();
}

void RStageGeometry::RenderSubmesh(RSubmesh *submesh)
{
}

}