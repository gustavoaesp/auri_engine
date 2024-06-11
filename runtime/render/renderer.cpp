#include "renderer.hpp"

#include "scene/scene.hpp"

#include "transform/matrix.hpp"

namespace eng
{

std::unique_ptr<RTextureManager> g_texture_manager;
std::unique_ptr<RMeshManager> g_mesh_manager;

RRenderer::RRenderer(std::unique_ptr<IRenderBackend>&& backend):
    backend_(std::move(backend))
{
    g_texture_manager = std::make_unique<RTextureManager>(backend_.get());
    g_mesh_manager = std::make_unique<RMeshManager>(backend_.get());

    descriptor_sets_buffer_.reserve(0x1000);
    descriptor_sets_texture_.reserve(0x1000);

    std::array<RDescriptorLayoutBinding, 2> layout_bindings_buffers = {
        RDescriptorLayoutBinding{
            .type = RDescriptorLayoutBindingType::kUniformBuffer,
            .bindingIndex = 0,
            .bindingStageAccessFlags = RDescriptorLayoutBindingStageAccess::EShaderStageVertexBit
        },
        RDescriptorLayoutBinding{
            .type = RDescriptorLayoutBindingType::kUniformBuffer,
            .bindingIndex =  1,
            .bindingStageAccessFlags = RDescriptorLayoutBindingStageAccess::EShaderStageVertexBit
        }
    };

    std::array<RDescriptorLayoutBinding, 2> layout_bindings_textures = {
        RDescriptorLayoutBinding{
            .type = RDescriptorLayoutBindingType::kTextureSampler,
            .bindingIndex = 0,
            .bindingStageAccessFlags = RDescriptorLayoutBindingStageAccess::EShaderStageFragmentBit
        },
        RDescriptorLayoutBinding{
            .type = RDescriptorLayoutBindingType::kTextureSampler,
            .bindingIndex = 1,
            .bindingStageAccessFlags = RDescriptorLayoutBindingStageAccess::EShaderStageFragmentBit
        }
    };

    desc_layout_buffers_ = std::unique_ptr<RDescriptorLayout>(
        backend_->CreateDescriptorLayout(
            layout_bindings_buffers.data(),
            layout_bindings_buffers.size()
        )
    );

    desc_layout_textures_ = std::unique_ptr<RDescriptorLayout>(
        backend_->CreateDescriptorLayout(
            layout_bindings_textures.data(),
            layout_bindings_textures.size()
        )
    );

    RTexture *output_tex = backend_->CreateImage2D(
        1600, 900, EFormat::kFormat_R8G8B8A8_UNORM
    );
    RTexture *output_depth = backend_->CreateImage2D(
        1600, 900, EFormat::kFormat_R24G8
    );

    std::array<RRenderPassAttachment, 1> rpass_attachments = {
        RRenderPassAttachment{
            .texture = output_tex,
            .load_op = RPassAttachmentLoadOp::kClear,
            .store_op = RPassAttachmentStoreOp::kStore
        }
    };
    RRenderPassAttachment rpass_depth_attachment = {
        .texture = output_depth,
        .load_op = RPassAttachmentLoadOp::kClear,
        .store_op = RPassAttachmentStoreOp::kStore
    };

    render_pass_geometry_ = std::unique_ptr<RRenderPass> (
        backend_->CreateRenderPass(
            rpass_attachments.data(),
            rpass_attachments.size(),
            &rpass_depth_attachment
        )
    );

    main_framebuffer_ = std::unique_ptr<RFramebuffer>(
        backend_->CreateFramebuffer(
            render_pass_geometry_.get(),
            &output_tex, 1, output_depth
        )
    );

    descriptor_pool_buffers_ = std::unique_ptr<RDescriptorPool>(
        backend_->CreateDescriptorPool(
            0x4000,
            RDescriptorLayoutBindingType::kUniformBuffer
        )
    );

    descriptor_pool_textures_ = std::unique_ptr<RDescriptorPool>(
        backend_->CreateDescriptorPool(
            0x4000,
            RDescriptorLayoutBindingType::kTextureSampler
        )
    );

    cmd_main_pool_ = std::unique_ptr<RCommandPool>(
        backend_->CreateCommandPool()
    );
    cmd_main_buffer_ = std::unique_ptr<RCommandBuffer>(
        cmd_main_pool_->CreateCommandBuffer(true)
    );

    main_vert_ = std::unique_ptr<RShader>(
        backend_->CreateShader(
            "shaders/main.vert.spv",
            RShaderPipelineBind::kShaderVertex
        )
    );
    main_frag_ = std::unique_ptr<RShader>(
        backend_->CreateShader(
            "shaders/main.frag.spv",
            RShaderPipelineBind::kShaderFragment
        )
    );

    RBlendState blend_state{};
    blend_state.num_blend_attachments = 1;

    RDepthStencilState depth_state{};
    depth_state.depth_test_enable = true;
    depth_state.depth_write_enable = true;
    depth_state.max_depth = 1.0f;

    pipeline_geometry_ = std::unique_ptr<RPipeline>(
        backend_->CreatePipeline(
            render_pass_geometry_.get(),
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

    main_sampler_ = std::unique_ptr<RSampler>(
        backend_->CreateSampler(
            RSamplerFilterMode::kFilterLinear,
            RSamplerAddressMode::kRepeat
        )
    );

    view_projection_uniform_ = std::unique_ptr<RBuffer> (
        backend_->CreateBuffer(
            RBufferUsage::kUniform,
            sizeof(mtx4f),
            nullptr
        )
    );
}

RRenderer::~RRenderer()
{
    backend_->Finalize();
}

void RRenderer::BeginFrame()
{
    backend_->BeginFrame();
    backend_->BeginRender();
}

void RRenderer::Render(const RScene& scene)
{
    mtx4f view_proj = CreateViewMatrix(
        vec3f(2.0f, 2.0f, -2.0f),
        vec3f(0.0f, 0.0f, 0.0f),
        vec3f(0.0f, 1.0f, 0.0f)
    );

    view_proj *= CreatePerspectiveProjectionMatrix(
        16.0f/9.0f, 60.0f, 0.1f, 100.0f
    );

    backend_->UpdateBuffer(
        view_projection_uniform_.get(),
        &view_proj,
        0,
        sizeof(mtx4f)
    );

    vec4f clear_colors[2] = {
        vec4f(0.0f, 0.0f, 0.0f, 1.0f),
        vec4f(1.0f, 1.0f, 1.0f, 1.0f)
    };

    cmd_main_buffer_->Reset();
    cmd_main_buffer_->BeginRecord();
    cmd_main_buffer_->CmdBeginRenderPass(
        render_pass_geometry_.get(),
        main_framebuffer_.get(),
        clear_colors,
        2,
        0x00,
        false
    );
    uint32_t count = 0;
    for (const auto& mesh : scene.scene_meshes) {
        mtx4f transform = CreateTranslateMatrix(mesh->position);
        transform *= mesh->rotation.toMatrix();
        transform *= CreateScaleMatrix(
            mesh->scale(0),
            mesh->scale(1),
            mesh->scale(2)
        );

        if (!mesh->uniform_buffer_transform) {
            mesh->uniform_buffer_transform = std::unique_ptr<RBuffer>(
                backend_->CreateBuffer(
                    RBufferUsage::kUniform,
                    sizeof(mtx4f), &transform
                )
            );
        } else {
            backend_->UpdateBuffer(
                mesh->uniform_buffer_transform.get(),
                &transform,
                0, sizeof(mtx4f)
            );
        }

        RDescriptorSet *descriptor_set_buffers = nullptr;
        RDescriptorSet *descriptor_set_textures = nullptr;
        if (count >= descriptor_sets_buffer_.size()) {
            descriptor_sets_buffer_.push_back(
                std::unique_ptr<RDescriptorSet>(
                    descriptor_pool_buffers_->AllocateSet(desc_layout_buffers_.get())
                )
            );
            descriptor_set_buffers = descriptor_sets_buffer_.back().get();
        } else {
            descriptor_set_buffers = descriptor_sets_buffer_[count].get();
        }
        if (count >= descriptor_sets_texture_.size()) {
            descriptor_sets_texture_.push_back(
                std::unique_ptr<RDescriptorSet>(
                    descriptor_pool_textures_->AllocateSet(desc_layout_textures_.get())
                )
            );
            descriptor_set_textures = descriptor_sets_texture_.back().get();
        } else {
            descriptor_set_textures = descriptor_sets_texture_[count].get();
        }
        count++;

        RTextureSamplerBinding sampler_binding{
            .texture = mesh->mesh->GetSubmesh(0)->material.diffuse.get(),
            .sampler = main_sampler_.get()
        };
        descriptor_set_textures->BindTextures(
            0, &sampler_binding, 1
        );
        std::array<RBufferBinding, 2> buffer_bindings = {
            RBufferBinding{
                .buffer = view_projection_uniform_.get(),
                .start_offset = 0,
                .size = sizeof(mtx4f)
            },
            RBufferBinding{
                .buffer = mesh->uniform_buffer_transform.get(),
                .start_offset = 0,
                .size = sizeof(mtx4f)
            }
        };
        descriptor_set_buffers->BindBuffers(
            0, buffer_bindings.data(), 2
        );
        cmd_main_buffer_->CmdBindPipeline(pipeline_geometry_.get());
        cmd_main_buffer_->CmdSetScissor(0, 0, 1600, 900);
        cmd_main_buffer_->CmdSetViewport(0, 0, 1600, 900);
        cmd_main_buffer_->CmdBindDescriptorSets(
            pipeline_geometry_.get(),
            (const RDescriptorSet**)std::array<RDescriptorSet*, 2>{
                descriptor_set_buffers,
                descriptor_set_textures
            }.data(),
            2
        );
        cmd_main_buffer_->CmdBindVertexBuffer(mesh->mesh->GetSubmesh(0)->vertex_buffer.get(), 0, 0);
        cmd_main_buffer_->CmdBindIndexBuffer(mesh->mesh->GetSubmesh(0)->index_buffer.get(), 0);
        cmd_main_buffer_->CmdDrawIndexed(
            mesh->mesh->GetSubmesh(0)->indices_count,
            0, 0
        );
    }
    cmd_main_buffer_->CmdEndRenderPass();
    cmd_main_buffer_->EndRecord();

    backend_->SubmitBuffers(
        std::array<RCommandBuffer*, 1>{cmd_main_buffer_.get()}.data(),
        1
    );
}

void RRenderer::Present()
{
    backend_->Present(
        main_framebuffer_.get()
    );
}



}