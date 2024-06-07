#ifndef _RENDER_BACKEND_VULKAN_HPP_
#define _RENDER_BACKEND_VULKAN_HPP_

#include <GLFW/glfw3.h>

#include "renderbackend.hpp"

#include "vk_mem_alloc.h"

#include "backend_vulkan/vk_device.hpp"
#include "backend_vulkan/synchronization/vk_semaphore.hpp"
#include "backend_vulkan/synchronization/vk_fence.hpp"
#include "backend_vulkan/transfer_buffers.hpp"
#include "render/texture_file.hpp"

namespace eng
{

class VulkanBuffer;
class VulkanSampler;
class VulkanShader;
class VulkanFramebuffer;
class VulkanRenderPass;
class VulkanPipeline;
class VulkanCommandPool;
class VulkanCommandBuffer;
class VulkanDescriptorSetLayout;
class VulkanDescriptorSet;
class VulkanDescriptorPool;

class VulkanRenderBackend : public IRenderBackend
{
public:
    VulkanRenderBackend(
        std::unique_ptr<VulkanInstance> &&vulkan_instance,
        GLFWwindow*, VkSurfaceKHR
    );
    ~VulkanRenderBackend() override;

    virtual void InitializeGUI() override;

    void BeginFrame() override;
    void BeginRender() override;

    void Present(RFramebuffer* final_image) override;

    RBuffer *CreateBuffer(
        RBufferUsage usage,
        size_t size,
        void *contents
    ) override;

    void UpdateBuffer(
        RBuffer *buffer, void *data,
        size_t start_offset, size_t size
    ) override;

    RTexture *CreateImage2D(
        uint32_t width, uint32_t height, EFormat pixel_fmt
    ) override;

    RTexture *CreateTexture2D(
        const RTextureFile *
    ) override;

    RRenderPass *CreateRenderPass(
        const RRenderPassAttachment *color_attachments, int num_color_attachments,
        const RRenderPassAttachment *depth
    ) override;

    RFramebuffer *CreateFramebuffer(
        RRenderPass *,
        RTexture **images, int num_images,
        RTexture *depth_stencil
    ) override;

    RPipeline *CreatePipeline(
        const RRenderPass *,
        const RBlendState *,
        const RDepthStencilState *depth_state,
        const RShader **shaders,
        int num_shaders,
        RVertexType vertex_type,
        RVertexType instance_type,
        RDescriptorLayout **,
        int num_descriptor_set_layouts
    ) override;

    RShader *CreateShader(const char *filename, RShaderPipelineBind) override;

    RCommandPool *CreateCommandPool() override;

    void SubmitBuffers(RCommandBuffer **, uint32_t num_buffers) override;

    RDescriptorLayout *CreateDescriptorLayout(
        const RDescriptorLayoutBinding *,
        uint32_t num_bindings
    ) override;

    RDescriptorPool *CreateDescriptorPool(
        uint32_t max_sets,
        RDescriptorLayoutBindingType
    ) override;

    RSampler *CreateSampler(
        RSamplerFilterMode,
        RSamplerAddressMode
    ) override;

    void Finalize() override;

private:
    std::unique_ptr<VulkanInstance> vulkan_instance_;
    std::unique_ptr<VulkanDevice> vulkan_device_;
    std::unique_ptr<VulkanSwapchain> vulkan_swapchain_;

    VmaAllocator vma_allocator_;

    std::unique_ptr<VulkanFence> in_flight_fence_;
    std::unique_ptr<VulkanSemaphore> image_available_semaphore_;
    std::unique_ptr<VulkanSemaphore> render_finished_semaphore_;

    std::vector<std::unique_ptr<VulkanFramebuffer>> presentation_framebuffers_;
    std::unique_ptr<VulkanRenderPass> presentation_render_pass_;
    std::unique_ptr<VulkanPipeline> presentation_pipeline_;
    std::unique_ptr<VulkanDescriptorSetLayout> presentation_descriptor_layout_;
    std::unique_ptr<VulkanDescriptorPool> presentation_descriptor_pool_;
    std::unique_ptr<VulkanDescriptorSet> presentation_descriptor_set_;

    std::unique_ptr<VulkanShader> presentation_shader_vert_;
    std::unique_ptr<VulkanShader> presentation_shader_frag_;

    std::unique_ptr<VulkanCommandPool> presentation_cmd_pool_;
    std::unique_ptr<VulkanCommandBuffer> presentation_cmd_buffer_;

    std::unique_ptr<VulkanBuffer> presentation_square_;
    std::unique_ptr<VulkanSampler> presentation_sampler_;

    std::unique_ptr<VulkanTransferBuffers> transfer_buffers_;

    VkSurfaceKHR win_surface_;

    uint32_t image_index_;

    bool gui_initialized_;
    VkDescriptorPool im_gui_descriptor_pool_;
};

}

#endif