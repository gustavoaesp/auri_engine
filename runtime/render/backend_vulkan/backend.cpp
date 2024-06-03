#include "backend_vulkan/backend.hpp"
#include "backend_vulkan/primitives/vk_buffer.hpp"
#include "backend_vulkan/primitives/vk_cmd_pool.hpp"
#include "backend_vulkan/primitives/vk_framebuffer.hpp"
#include "backend_vulkan/primitives/vk_pipeline.hpp"
#include "backend_vulkan/primitives/vk_render_pass.hpp"
#include "backend_vulkan/primitives/vk_sampler.hpp"
#include "backend_vulkan/primitives/vk_shader.hpp"
#include "backend_vulkan/vertex_input_desc.hpp"
#include "backend_vulkan/vk_swapchain.hpp"
#include "backend.hpp"

#include <string.h>

namespace eng
{

static Vertex screen_square[6] = {
    {vec3f(-1.0f,-1.0f, 0.0f), vec4f(0.0f, 1.0f, 0.0f, 0.0f)},
    {vec3f(-1.0f, 1.0f, 0.0f), vec4f(0.0f, 0.0f, 0.0f, 0.0f)},
    {vec3f( 1.0f,-1.0f, 0.0f), vec4f(1.0f, 1.0f, 0.0f, 0.0f)},
    {vec3f( 1.0f,-1.0f, 0.0f), vec4f(1.0f, 1.0f, 0.0f, 0.0f)},
    {vec3f(-1.0f, 1.0f, 0.0f), vec4f(0.0f, 0.0f, 0.0f, 0.0f)},
    {vec3f( 1.0f, 1.0f, 0.0f), vec4f(1.0f, 0.0f, 0.0f, 0.0f)}
};

VulkanRenderBackend::VulkanRenderBackend(
    std::unique_ptr<VulkanInstance> &&vulkan_instance, GLFWwindow* window, VkSurfaceKHR win_surface
):
    vulkan_instance_(std::move(vulkan_instance)), win_surface_(win_surface)
{
    vulkan_device_ = std::make_unique<VulkanDevice>(
        *vulkan_instance_,
        win_surface
    );

    vulkan_swapchain_ = std::make_unique<VulkanSwapchain>(
        *vulkan_instance_, vulkan_device_.get(), window, win_surface
    );

    image_available_semaphore_ = std::make_unique<VulkanSemaphore>(vulkan_device_->get());
    render_finished_semaphore_ = std::make_unique<VulkanSemaphore>(vulkan_device_->get());
    in_flight_fence_ = std::make_unique<VulkanFence>(vulkan_device_->get(), true);

    valloc_ = std::make_unique<VAlloc>(
        vulkan_device_.get(),
        vulkan_device_->GetMemoryProperties()
    );

    presentation_render_pass_ = std::make_unique<VulkanRenderPass>(
        vulkan_device_->get(),
        vulkan_swapchain_.get(),
        false
    );

    RDescriptorLayoutBinding bindings{
        .type = RDescriptorLayoutBindingType::kTextureSampler,
        .bindingIndex = 0,
        .bindingStageAccessFlags = EShaderStageFragmentBit
    };

    presentation_descriptor_layout_ = std::make_unique<VulkanDescriptorSetLayout>(
        vulkan_device_->get(),
        &bindings,
        1
    );
    presentation_descriptor_pool_ = std::make_unique<VulkanDescriptorPool>(
        vulkan_device_->get(), 16,
        RDescriptorLayoutBindingType::kTextureSampler
    );
    presentation_descriptor_set_ = std::make_unique<VulkanDescriptorSet>(
        vulkan_device_->get(),
        presentation_descriptor_pool_->Allocate(presentation_descriptor_layout_.get())
    );
    RBlendState presentation_blend{};
    presentation_blend.num_blend_attachments = 1;
    presentation_shader_vert_ = std::make_unique<VulkanShader>(
        vulkan_device_->get(), "shaders/presentation.vert.spv",
        RShaderPipelineBind::kShaderVertex
    );
    presentation_shader_frag_ = std::make_unique<VulkanShader>(
        vulkan_device_->get(), "shaders/presentation.frag.spv",
        RShaderPipelineBind::kShaderFragment
    );

    std::array<RDescriptorLayout*, 1> presentation_layouts = {
        presentation_descriptor_layout_.get()
    };
    std::array<RShader*, 2> shaders = {
        presentation_shader_vert_.get(),
        presentation_shader_frag_.get()
    };
    presentation_pipeline_ = std::unique_ptr<VulkanPipeline>(
        (VulkanPipeline*)CreatePipeline(
            presentation_render_pass_.get(),
            &presentation_blend,
            nullptr,
            (const RShader**)shaders.data(),
            2,
            RVertexType::kVertexPos3Col4, RVertexType::kNoFormat,
            presentation_layouts.data(), 1
        )
    );
    for (int i = 0; i < vulkan_swapchain_->getImageCount(); ++i) {
        presentation_framebuffers_.push_back(
            std::make_unique<VulkanFramebuffer>(
                vulkan_device_->get(), vulkan_swapchain_.get(), nullptr, i,
                presentation_render_pass_.get()
            )
        );
    }

    presentation_square_ = std::unique_ptr<VulkanBuffer>(
        static_cast<VulkanBuffer*>(
            CreateBuffer(
                RBufferUsage::kVertex,
                sizeof(Vertex) * 6,
                screen_square
            )
        )
    );

    RSamplerAttributes presentation_sampler_attrs{
        .filter_mode = RSamplerFilterMode::kFilterLinear,
        .address_mode = RSamplerAddressMode::kRepeat
    };

    presentation_sampler_ = std::make_unique<VulkanSampler>(
        vulkan_device_->get(),
        &presentation_sampler_attrs
    );

    presentation_cmd_pool_ = std::make_unique<VulkanCommandPool>(vulkan_device_->get());
    presentation_cmd_buffer_ = std::unique_ptr<VulkanCommandBuffer>(
        static_cast<VulkanCommandBuffer*>(presentation_cmd_pool_->CreateCommandBuffer(true))
    );
}

VulkanRenderBackend::~VulkanRenderBackend()
{
}

void VulkanRenderBackend::BeginFrame()
{
}

void VulkanRenderBackend::BeginRender()
{
    VkFence flight_fence = in_flight_fence_->GetVkHandle();
    vkWaitForFences(
        vulkan_device_->get(),
        1,
        &flight_fence,
        VK_TRUE,
        UINT64_MAX
    );
    vkResetFences(
        vulkan_device_->get(),
        1,
        &flight_fence
    );
    vkAcquireNextImageKHR(
        vulkan_device_->get(),
        vulkan_swapchain_->get(),
        UINT64_MAX,
        image_available_semaphore_->GetVkHandle(),
        VK_NULL_HANDLE,
        &image_index_
    );
}

void VulkanRenderBackend::Present(RFramebuffer *final_image)
{
    vec4f clearColor(0.0f,0.0f,0.0f,1.0f);
    RTextureSamplerBinding tex_bind{
        .texture = final_image->GetImage(0),
        .sampler = presentation_sampler_.get()
    };

    std::array<RDescriptorSet*, 1> descriptor_sets_array = {
        presentation_descriptor_set_.get()
    };
    presentation_descriptor_set_->BindTextures(
        0, &tex_bind, 1
    );
    presentation_cmd_buffer_->BeginRecord();
    presentation_cmd_buffer_->CmdBeginRenderPass(
        presentation_render_pass_.get(),
        presentation_framebuffers_[image_index_].get(),
        &clearColor, 1, 0x00, false
    );
    presentation_cmd_buffer_->CmdBindPipeline(presentation_pipeline_.get());
    presentation_cmd_buffer_->CmdSetScissor(
        0, 0,
        presentation_framebuffers_[image_index_]->GetWidth(),
        presentation_framebuffers_[image_index_]->GetHeight()
    );
    presentation_cmd_buffer_->CmdSetViewport(
        0, 0,
        presentation_framebuffers_[image_index_]->GetWidth(),
        presentation_framebuffers_[image_index_]->GetHeight()
    );
    presentation_cmd_buffer_->CmdBindVertexBuffer(presentation_square_.get(), 0, 0);
    presentation_cmd_buffer_->CmdBindDescriptorSets(
        presentation_pipeline_.get(),
        (const RDescriptorSet**)descriptor_sets_array.data(),
        1
    );
    presentation_cmd_buffer_->CmdDraw(6, 0);
    presentation_cmd_buffer_->CmdEndRenderPass();
    presentation_cmd_buffer_->EndRecord();

    vulkan_device_->GraphicsSubmit(
        presentation_cmd_buffer_.get(),
        nullptr,
        render_finished_semaphore_.get(),
        EStageWait::kStageColorAttachment,
        in_flight_fence_.get()
    );

    vulkan_device_->PresentSubmit(
        *vulkan_swapchain_,
        render_finished_semaphore_.get(),
        &image_index_
    );
}

RBuffer *VulkanRenderBackend::CreateBuffer(RBufferUsage usage, size_t size, void *contents)
{
    VulkanBuffer *new_buffer = new VulkanBuffer(
        vulkan_device_->get(),
        valloc_.get(),
        usage,
        size
    );

    if (contents) {
        void *mapped;
        vkMapMemory(vulkan_device_->get(), new_buffer->vk_dev_mem, 0, size, 0, &mapped);
        memcpy(mapped, contents, size);
        vkUnmapMemory(vulkan_device_->get(), new_buffer->vk_dev_mem);
    }

    return new_buffer;
}

void VulkanRenderBackend::UpdateBuffer(RBuffer *buffer, void *data, size_t start_offset, size_t size)
{
}

RTexture *VulkanRenderBackend::CreateTexture2D(uint32_t width, uint32_t height, EFormat pixel_fmt, void *contents)
{
    VulkanTexture *texture = new VulkanTexture(vulkan_device_->get(), pixel_fmt, width, height);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    const auto& fmt_map_pair = fmt_map.find(pixel_fmt);
    if (fmt_map_pair == fmt_map.end()) {
        // TODO error
    }
    imageInfo.format = fmt_map_pair->second;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT
                    | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                    | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(vulkan_device_->get(), &imageInfo, nullptr, &texture->vk_image) != VK_SUCCESS) {
        // TODO error
    }

    valloc_->AllocateTextureImage(texture->vk_image, &texture->vk_dev_memory, EMemoryLocation::kMemoryLocationDevice);

    vkBindImageMemory(vulkan_device_->get(), texture->vk_image, texture->vk_dev_memory, 0);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture->vk_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = fmt_map_pair->second;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vulkan_device_->get(), &viewInfo, nullptr, &texture->vk_image_view) != VK_SUCCESS) {
        // TODO error
    }

    return texture;
}

RRenderPass *VulkanRenderBackend::CreateRenderPass(const RRenderPassAttachment *color_attachments, int num_color_attachments, const RRenderPassAttachment *depth)
{
    return new VulkanRenderPass(
        vulkan_device_->get(),
        color_attachments,
        num_color_attachments,
        depth
    );
}

RFramebuffer *VulkanRenderBackend::CreateFramebuffer(
    RRenderPass *render_pass, RTexture **images,
    int num_images, RTexture *depth_stencil)
{
    return new VulkanFramebuffer(
        vulkan_device_->get(),
        static_cast<VulkanRenderPass*>(render_pass),
        (VulkanTexture**)images,
        num_images,
        static_cast<VulkanTexture*>(depth_stencil)
    );
}

RPipeline *VulkanRenderBackend::CreatePipeline(
    const RRenderPass *render_pass,
    const RBlendState *blend_state,
    const RDepthStencilState *depth_state,
    const RShader **shaders, int num_shaders,
    RVertexType vertex_type, RVertexType instance_type,
    RDescriptorLayout **descriptor_set_layouts, int num_descriptor_set_layouts)
{
    VulkanVertexDescription vertex_description = VulkanBuildDescriptors(vertex_type, instance_type);

    return new VulkanPipeline(
        vulkan_device_->get(),
        static_cast<const VulkanRenderPass*>(render_pass),
        blend_state,
        depth_state,
        (const VulkanShader**)shaders,
        num_shaders,
        &vertex_description,
        (const VulkanDescriptorSetLayout**)descriptor_set_layouts,
        num_descriptor_set_layouts
    );
}

RShader *VulkanRenderBackend::CreateShader(const char *filename, RShaderPipelineBind pipeline_bind)
{
    return new VulkanShader(vulkan_device_->get(), filename, pipeline_bind);
}

RCommandPool *VulkanRenderBackend::CreateCommandPool()
{
    return new VulkanCommandPool(vulkan_device_->get());
}

void VulkanRenderBackend::SubmitBuffers(RCommandBuffer **buffers, uint32_t num_buffers)
{
    std::array<VkCommandBuffer, 64> vk_buffers_array{};

    for (int i = 0; i < num_buffers; ++i) {
        VulkanCommandBuffer *vk_buffer = static_cast<VulkanCommandBuffer*>(buffers[i]);
        vk_buffers_array[i] = vk_buffer->vk_command_buffer;
    }

    vulkan_device_->GraphicsSubmit(
        image_available_semaphore_.get(),
        nullptr,
        EStageWait::kStageColorAttachment,
        nullptr,
        vk_buffers_array.data(),
        num_buffers
    );
}

RDescriptorLayout *VulkanRenderBackend::CreateDescriptorLayout(const RDescriptorLayoutBinding *, uint32_t num_bindings)
{
    return nullptr;
}

RDescriptorPool *VulkanRenderBackend::CreateDescriptorPool(uint32_t max_sets, RDescriptorLayoutBindingType)
{
    return nullptr;
}

void VulkanRenderBackend::Finalize()
{
    vulkan_device_->WaitIdle();
}

}