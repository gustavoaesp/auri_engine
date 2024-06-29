#include "core/global_context.hpp"

#include "scene/scene.hpp"
#include "stages/render_stage_lighting.hpp"
#include "vertex.hpp"

#include "renderer.hpp"

namespace eng
{
Vertex quad[4] = {
    {vec3f(-1.0f, -1.0f, 0.0f), vec4f(0.0f, 0.0f, 0.0f, 0.0f)},
    {vec3f( 1.0f, -1.0f, 0.0f), vec4f(1.0f, 0.0f, 0.0f, 0.0f)},
    {vec3f( 1.0f,  1.0f, 0.0f), vec4f(1.0f, 1.0f, 0.0f, 0.0f)},
    {vec3f(-1.0f,  1.0f, 0.0f), vec4f(0.0f, 1.0f, 0.0f, 0.0f)}
};

uint32_t indices[6] = {
    0, 3, 1, 2, 1, 3
};

RStageLighting::RStageLighting(IRenderBackend *backend, RFramebuffer *gbuffer):
    IRenderStage(
        backend, 0x4000,
        RDescriptorLayoutBindingStageAccess::EShaderStageFragmentBit,
        RDescriptorLayoutBindingStageAccess::EShaderStageFragmentBit
    ),
    backend_ref_(backend), gbuffer_ref_(gbuffer)
{
    RBlendState blend_state;
    blend_state.num_blend_attachments = 1;
    blend_state.blend_attachments[0].blendEnable = true;
    blend_state.blend_attachments[0].colorBlendOp = RBlendOp::kOpAdd;
    blend_state.blend_attachments[0].alphaBlendOp = RBlendOp::kOpMax;
    blend_state.blend_attachments[0].dstColorBlendFactor = RBlendFactor::kOne;
    blend_state.blend_attachments[0].srcColorBlendFactor = RBlendFactor::kOne;
    for (int i = 0; i < 4; ++i) {
        blend_state.blend_constants[i] = 1.0f;
    }

    std::array<RDescriptorLayoutBinding, 4> buffer_bindings;
    std::array<RDescriptorLayoutBinding, 8> texture_bindings;
    for (int i = 0; i < buffer_bindings.size(); ++i) {
        buffer_bindings[i].bindingIndex = i;
        buffer_bindings[i].bindingStageAccessFlags =
            RDescriptorLayoutBindingStageAccess::EShaderStageFragmentBit;
        buffer_bindings[i].type = RDescriptorLayoutBindingType::kUniformBuffer;
    }
    for (int i = 0; i < texture_bindings.size(); ++i) {
        texture_bindings[i].bindingIndex = i;
        texture_bindings[i].bindingStageAccessFlags =
            RDescriptorLayoutBindingStageAccess::EShaderStageFragmentBit;
        texture_bindings[i].type = RDescriptorLayoutBindingType::kTextureSampler;
    }

    RTexture *output_texture = backend->CreateImage2D(
        1600, 900,
        EFormat::kFormat_R8G8B8A8_UNORM
    );

    RRenderPassAttachment output_attachment;
    output_attachment.texture = output_texture;
    output_attachment.load_op = RPassAttachmentLoadOp::kClear;
    output_attachment.store_op = RPassAttachmentStoreOp::kStore;

    render_pass_ = std::unique_ptr<RRenderPass>(
        backend->CreateRenderPass(
            &output_attachment,
            1, nullptr
        )
    );
    output_frame_ = std::unique_ptr<RFramebuffer>(
        backend->CreateFramebuffer(
            render_pass_.get(),
            std::array<RTexture*, 1>{output_texture}.data(), 1,
            nullptr
        )
    );

    directional_vertex_shader_ = std::unique_ptr<RShader>(
        backend->CreateShader("shaders/light.vert.spv", RShaderPipelineBind::kShaderVertex)
    );
    directional_pixel_shader_ = std::unique_ptr<RShader>(
        backend->CreateShader("shaders/dirlight.frag.spv", RShaderPipelineBind::kShaderFragment)
    );
    directional_pipeline_ = std::unique_ptr<RPipeline>(
        backend->CreatePipeline(
            render_pass_.get(),
            &blend_state, nullptr,
            (const RShader**)std::array<RShader *, 2>{
                directional_vertex_shader_.get(),
                directional_pixel_shader_.get()
            }.data(), 2,
            RVertexType::kVertexPos3Col4,
            RVertexType::kNoFormat,
            std::array<RDescriptorLayout*, 2>{
                descriptor_layout_buffers_.get(),
                descriptor_layout_textures_.get()
            }.data(), 2
        )
    );

    ambient_pipeline_ = std::unique_ptr<RPipeline>(
        backend->CreatePipeline(
            render_pass_.get(),
            &blend_state, nullptr,
            (const RShader**)std::array<RShader*, 2>{
                g_context->shader_list->Get("shaders/ambientlight.frag.spv"),
                g_context->shader_list->Get("shaders/light.vert.spv")
            }.data(),
            2,
            RVertexType::kVertexPos3Col4,
            RVertexType::kNoFormat,
            std::array<RDescriptorLayout*, 2> {
                descriptor_layout_buffers_.get(),
                descriptor_layout_textures_.get()
            }.data(), 2
        )
    );

    cmd_pool_ = std::unique_ptr<RCommandPool>(
        backend->CreateCommandPool()
    );
    cmd_buffer_ = std::unique_ptr<RCommandBuffer>(
        cmd_pool_->CreateCommandBuffer(true)
    );

    main_sampler_ = std::unique_ptr<RSampler>(
        backend->CreateSampler(
            RSamplerFilterMode::kFilterLinear,
            RSamplerAddressMode::kRepeat
        )
    );

    quad_vertex_buffer_ = std::unique_ptr<RBuffer>(
        backend->CreateBuffer(
            RBufferUsage::kVertex, sizeof(Vertex) * 6, quad
        )
    );
    quad_index_buffer_ = std::unique_ptr<RBuffer>(
        backend->CreateBuffer(
            RBufferUsage::kIndex, sizeof(uint32_t) * 6, indices
        )
    );

    ambient_uniform_ = std::unique_ptr<RBuffer>(
        backend->CreateBuffer(
            RBufferUsage::kUniform,
            sizeof(vec4f),
            nullptr
        )
    );
}

RStageLighting::~RStageLighting()
{}

void RStageLighting::Render(RScene &scene)
{
    ResetCounters();

    RDescriptorSet *descriptor_set_textures = NextSet(RDescriptorLayoutBindingType::kTextureSampler);
    std::array<RTextureSamplerBinding, 2> texture_bindings{};
    for (int i = 0; i < 2; ++i) {
        texture_bindings[i].texture = gbuffer_ref_->GetImage(i);
        texture_bindings[i].sampler = main_sampler_.get();
    }
    descriptor_set_textures->BindTextures(0, texture_bindings.data(), texture_bindings.size());

    vec4f clear_color(0.0f, 0.0f, 0.0f, 1.0f);
    cmd_buffer_->Reset();
    cmd_buffer_->BeginRecord();
    cmd_buffer_->CmdBeginRenderPass(
        render_pass_.get(),
        output_frame_.get(),
        &clear_color, 1,
        0x00, false
    );

    ProcessAmbientLight(
        scene.ambient_color,
        NextSet(RDescriptorLayoutBindingType::kUniformBuffer),
        descriptor_set_textures
    );

    for (auto &light : scene.scene_lights) {
        RDescriptorSet *descriptor_set_buffers = NextSet(RDescriptorLayoutBindingType::kUniformBuffer);

        switch(light->type) {
        case RSceneLightType::kLightDirectional:
            ProcessDirectionalLight(
                light.get(),
                descriptor_set_buffers,
                descriptor_set_textures
            );
            break;
        }
    }
    cmd_buffer_->CmdEndRenderPass();
    cmd_buffer_->EndRecord();
}

void RStageLighting::ProcessDirectionalLight(
    RSceneLight *light, RDescriptorSet *buffers, RDescriptorSet *textures)
{
    RLightDirectionalUniform uniform_data;
    uniform_data.direction = light->direction;
    uniform_data.color = light->color;
    uniform_data.intensity = light->intensity;

    if (!light->uniform_buffer) {
        light->uniform_buffer = std::unique_ptr<RBuffer>(
            backend_ref_->CreateBuffer(RBufferUsage::kUniform, sizeof(RLightDirectionalUniform), &uniform_data)
        );
    } else {
        backend_ref_->UpdateBuffer(
            light->uniform_buffer.get(),
            &uniform_data,
            0, sizeof(RLightDirectionalUniform)
        );
    }

    RBufferBinding buffer_binding{
        .buffer = light->uniform_buffer.get(),
        .start_offset = 0,
        .size = sizeof(RLightDirectionalUniform),
    };
    buffers->BindBuffers(0, &buffer_binding, 1);

    cmd_buffer_->CmdBindPipeline(directional_pipeline_.get());
    cmd_buffer_->CmdBindDescriptorSets(
        directional_pipeline_.get(),
        (const RDescriptorSet**)std::array<RDescriptorSet*, 2>{
            buffers, textures
        }.data(),
        2
    );
    cmd_buffer_->CmdSetScissor(0, 0, 1600, 900);
    cmd_buffer_->CmdSetViewport(0, 0, 1600, 900);
    cmd_buffer_->CmdBindVertexBuffer(quad_vertex_buffer_.get(), 0, 0);
    cmd_buffer_->CmdBindIndexBuffer(quad_index_buffer_.get(), 0);
    cmd_buffer_->CmdDrawIndexed(6, 0, 0);
}

void RStageLighting::ProcessAmbientLight(
    const vec3f &color,
    RDescriptorSet *buffers, RDescriptorSet *textures)
{
    backend_ref_->UpdateBuffer(
        ambient_uniform_.get(),
        (void*)&color, 0,
        sizeof(vec3f)
    );
    RBufferBinding buffer_binding{
        .buffer = ambient_uniform_.get(),
        .start_offset = 0,
        .size = sizeof(vec4f)
    };

    buffers->BindBuffers(0, &buffer_binding, 1);
    cmd_buffer_->CmdBindPipeline(ambient_pipeline_.get());
    cmd_buffer_->CmdBindDescriptorSets(
        ambient_pipeline_.get(),
        (const RDescriptorSet**)std::array<RDescriptorSet*, 2>{
            buffers,
            textures
        }.data(), 2
    );
    cmd_buffer_->CmdSetScissor(0, 0, 1600, 900);
    cmd_buffer_->CmdSetViewport(0, 0, 1600, 900);
    cmd_buffer_->CmdBindVertexBuffer(quad_vertex_buffer_.get(), 0, 0);
    cmd_buffer_->CmdBindIndexBuffer(quad_index_buffer_.get(), 0);

    cmd_buffer_->CmdDrawIndexed(6, 0, 0);
}

}