#include "backend_vulkan/primitives/vk_framebuffer.hpp"
#include "backend_vulkan/primitives/vk_texture.hpp"

namespace eng
{

VulkanFramebuffer::VulkanFramebuffer(
    VkDevice vk_device,
    const VulkanSwapchain* swapchain, VulkanTexture *depth_stencil,
    uint32_t imageIndex, const VulkanRenderPass *renderPass):
    vk_device_ref(vk_device), RFramebuffer(nullptr, 0, depth_stencil)
{
    std::array<VkImageView, 2> attachments = {
        swapchain->getImageView(imageIndex),
        (depth_stencil) ? depth_stencil->vk_image_view : VK_NULL_HANDLE
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->vk_render_pass;
    framebufferInfo.attachmentCount = (depth_stencil)? 2 : 1;
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width_ = swapchain->getExtent().width;
    framebufferInfo.height = height_ = swapchain->getExtent().height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(vk_device, &framebufferInfo, nullptr, &vk_framebuffer)
        != VK_SUCCESS)
    {
        // TODO error
    }
}

VulkanFramebuffer::VulkanFramebuffer(
    VkDevice vk_device, VulkanRenderPass *render_pass,
    VulkanTexture **textures, int num_textures, VulkanTexture* depth_stencil):
    vk_device_ref(vk_device), RFramebuffer((RTexture**)textures, num_textures, depth_stencil)
{
    VkFramebufferCreateInfo vk_framebuffer_info{};
    std::vector<VkImageView> vk_attachments;
    vk_attachments.reserve(num_textures + (depth_stencil != nullptr));

    for (int i = 0; i < num_textures; ++i) {
        vk_attachments.push_back(textures[i]->vk_image_view);
    }
    if (depth_stencil) {
        vk_attachments.push_back(depth_stencil->vk_image_view);
    }

    vk_framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vk_framebuffer_info.renderPass = render_pass->vk_render_pass;
    vk_framebuffer_info.attachmentCount = (depth_stencil) ? num_textures + 1 : num_textures;
    vk_framebuffer_info.pAttachments = vk_attachments.data();
    vk_framebuffer_info.width = width_;
    vk_framebuffer_info.height = height_;
    vk_framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(vk_device, &vk_framebuffer_info, nullptr, &vk_framebuffer) != VK_SUCCESS) {
        return;
    }
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    if(vk_framebuffer) {
        vkDestroyFramebuffer(vk_device_ref, vk_framebuffer, nullptr);
    }
}

}
