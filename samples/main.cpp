#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <backend_vulkan/instance.hpp>
#include <backend_vulkan/backend.hpp>
#include <render/vertex.hpp>
#include <render/transform/matrix.hpp>
#include <render/texture_file.hpp>

eng::Vertex my_triangle[3] = {
    {eng::vec3f( 0.0f, 1.0f, 0.0f), eng::vec4f(0.5f, 1.0f, 0.0f, 1.0f)},
    {eng::vec3f( 1.0f,-1.0f, 0.0f), eng::vec4f(1.0f, 0.0f, 0.0f, 1.0f)},
    {eng::vec3f(-1.0f,-1.0f, 0.0f), eng::vec4f(0.0f, 0.0f, 1.0f, 1.0f)}
};

eng::RBlendState blend_state{};

struct UniformContents
{
    eng::mtx4f mvp;
};

int main(int argc, char** argv)
{
    if (!glfwInit()) {
        std::cerr << "Error initializing glfw\n";
        return -1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    std::unique_ptr<eng::VulkanInstance> vk_instance;
    vk_instance = std::make_unique<eng::VulkanInstance>(
        std::vector<const char*>{},
        std::vector<const char*>{"VK_LAYER_KHRONOS_validation"}
    );

    blend_state.num_blend_attachments = 1;
    blend_state.blend_attachments[0].blendEnable = false;

    eng::RTextureFile tex_file = eng::RTextureRead("textures/texture.tex");

    {
        VkSurfaceKHR win_surface;
        GLFWwindow *window = glfwCreateWindow(1600, 900, "Test", nullptr, nullptr);
        glfwCreateWindowSurface(vk_instance->get(), window, nullptr, &win_surface);
        std::unique_ptr<eng::VulkanRenderBackend> backend = std::make_unique<eng::VulkanRenderBackend>(
            std::move(vk_instance),
            window,
            win_surface
        );
        backend->InitializeGUI();
        {
            eng::RDescriptorLayoutBinding binding_buffer{
                .type = eng::RDescriptorLayoutBindingType::kUniformBuffer,
                .bindingIndex = 0,
                .bindingStageAccessFlags = eng::RDescriptorLayoutBindingStageAccess::EShaderStageVertexBit
            };
            eng::RDescriptorLayoutBinding binding_tex{
                .type = eng::RDescriptorLayoutBindingType::kTextureSampler,
                .bindingIndex = 0,
                .bindingStageAccessFlags =eng::RDescriptorLayoutBindingStageAccess::EShaderStageFragmentBit
            };
            eng::RTexture *output_tex = backend->CreateImage2D(
                1600, 900,
                eng::EFormat::kFormat_R8G8B8A8_UNORM
            );
            eng::RRenderPassAttachment rpass_attachment[] = {{
                .texture = output_tex,
                .load_op = eng::RPassAttachmentLoadOp::kClear,
                .store_op = eng::RPassAttachmentStoreOp::kStore
            }};
            std::unique_ptr<eng::RRenderPass> render_pass(
                backend->CreateRenderPass(
                    rpass_attachment, 1,
                    nullptr
                )
            );
            std::unique_ptr<eng::RDescriptorLayout> desc_layout_buffers(
                backend->CreateDescriptorLayout(
                    &binding_buffer,
                    1
                )
            );
            std::unique_ptr<eng::RDescriptorLayout> desc_layout_tex(
                backend->CreateDescriptorLayout(
                    &binding_tex,
                    1
                )
            );
            std::unique_ptr<eng::RDescriptorPool> descriptor_pool_buffer(
                backend->CreateDescriptorPool(
                    64, eng::RDescriptorLayoutBindingType::kUniformBuffer
                )
            );
            std::unique_ptr<eng::RDescriptorPool> descriptor_pool_tex(
                backend->CreateDescriptorPool(
                    64, eng::RDescriptorLayoutBindingType::kTextureSampler
                )
            );
            std::unique_ptr<eng::RDescriptorSet> desc_set_buffers(
                descriptor_pool_buffer->AllocateSet(desc_layout_buffers.get())
            );
            std::unique_ptr<eng::RDescriptorSet> desc_set_tex(
                descriptor_pool_tex->AllocateSet(desc_layout_tex.get())
            );
            std::unique_ptr<eng::RShader> vtx_shader(
                backend->CreateShader("shaders/vert.vert.spv", eng::RShaderPipelineBind::kShaderVertex)
            );
            std::unique_ptr<eng::RShader> frg_shader(
                backend->CreateShader("shaders/frag.frag.spv", eng::RShaderPipelineBind::kShaderFragment)
            );
            std::array<const eng::RShader*, 2> shaders = {
                vtx_shader.get(), frg_shader.get()
            };
            std::unique_ptr<eng::RSampler> sampler(
                backend->CreateSampler(
                    eng::RSamplerFilterMode::kFilterLinear,
                    eng::RSamplerAddressMode::kRepeat
                )
            );
            std::unique_ptr<eng::RPipeline> pipeline(
                backend->CreatePipeline(
                    render_pass.get(),
                    &blend_state,
                    nullptr,
                    shaders.data(),
                    2,
                    eng::RVertexType::kVertexPos3Col4,
                    eng::RVertexType::kNoFormat,
                    std::array<eng::RDescriptorLayout*, 2>{
                        desc_layout_buffers.get(),
                        desc_layout_tex.get()
                    }.data(),
                    2
                )
            );
            std::array<eng::RTexture*, 1> textures = {output_tex};
            std::unique_ptr<eng::RFramebuffer> framebuffer(
                backend->CreateFramebuffer(
                    render_pass.get(),
                    textures.data(),
                    1,
                    nullptr
                )
            );
            std::unique_ptr<eng::RBuffer> vbuffer(
                backend->CreateBuffer(
                    eng::RBufferUsage::kVertex,
                    sizeof(eng::Vertex) * 3,
                    my_triangle
                )
            );
            std::unique_ptr<eng::RCommandPool> cmd_pool(
                backend->CreateCommandPool()
            );
            std::unique_ptr<eng::RCommandBuffer> cmd_buff(
                cmd_pool->CreateCommandBuffer(true)
            );
            std::unique_ptr<eng::RBuffer> uniform_buffer(
                backend->CreateBuffer(
                    eng::RBufferUsage::kUniform,
                    sizeof(eng::mtx4f) * 2,
                    nullptr
                )
            );
            std::unique_ptr<eng::RTexture> texture(
                backend->CreateTexture2D(&tex_file)
            );
            UniformContents uniforms;

            eng::vec4f clear(0.0f, 0.0f, 0.0f, 1.0f);

            float angle = 0;

            while (!glfwWindowShouldClose(window)) {
                glfwSwapBuffers(window);
                backend->BeginFrame();
                backend->BeginRender();
                eng::RBufferBinding binding_buffer{
                    .buffer = uniform_buffer.get(),
                    .start_offset = 0,
                    .size = sizeof(UniformContents)
                };
                eng::RTextureSamplerBinding binding_texture{
                    .texture = texture.get(),
                    .sampler = sampler.get()
                };
                desc_set_buffers->BindBuffers(0, &binding_buffer, 1);
                desc_set_tex->BindTextures(0, &binding_texture, 1);
                uniforms.mvp = eng::CreateRotationMatrixZ(angle);
                uniforms.mvp *= eng::CreateViewMatrix(
                    eng::vec3f(1.0f,1.0f, -2.0f), eng::vec3f(0.0f,0.0f,0.0f),
                    eng::vec3f(0.0f, 1.0f, 0.0f)
                );
                uniforms.mvp *= eng::CreatePerspectiveProjectionMatrix(
                    16.0f/9.0f, 60.0f, 0.1f, 100.0f
                );
                //uniforms.mvp = eng::CreateScaleMatrix(0.4, 0.4, 0.4);
                backend->UpdateBuffer(uniform_buffer.get(), &uniforms, 0, sizeof(eng::mtx4f));
                //cmd_buff->Reset();
                cmd_buff->BeginRecord();

                ImGui::Begin("Rotation");
                ImGui::Text("Rotation slider");
                ImGui::SliderAngle("Rot X", &angle);
                ImGui::End();


                cmd_buff->CmdBeginRenderPass(
                    render_pass.get(),
                    framebuffer.get(),
                    &clear,
                    1,
                    0x00,
                    false
                );
                cmd_buff->CmdBindPipeline(pipeline.get());
                cmd_buff->CmdBindDescriptorSets(
                    pipeline.get(),
                    (const eng::RDescriptorSet**)std::array<eng::RDescriptorSet*, 2>{
                        desc_set_buffers.get(),
                        desc_set_tex.get()
                    }.data(),
                    2);
                cmd_buff->CmdSetViewport(0, 0, 1600, 900);
                cmd_buff->CmdSetScissor(0, 0, 1600, 900);
                cmd_buff->CmdBindVertexBuffer(vbuffer.get(), 0, 0);
                cmd_buff->CmdDraw(3, 0);
                cmd_buff->CmdEndRenderPass();
                cmd_buff->EndRecord();

                std::array<eng::RCommandBuffer*, 1> cmd_buffers = { cmd_buff.get() };
                backend->SubmitBuffers(cmd_buffers.data(), 1);

                backend->Present(framebuffer.get());

                glfwPollEvents();
                //angle+=0.01f;
            }

            backend->Finalize();
            cmd_buff->Reset();
        }
        backend.reset();
        glfwDestroyWindow(window);
    }

    glfwTerminate();
    return 0;
}