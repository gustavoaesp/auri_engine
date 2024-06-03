#ifndef _RENDER_BACKEND_VULKAN_FRAMEBUFFER_HPP_
#define _RENDER_BACKEND_VULKAN_FRAMEBUFFER_HPP_

#include "render/primitives/framebuffer.hpp"

#include "backend_vulkan/primitives/vk_render_pass.hpp"
#include "backend_vulkan/primitives/vk_texture.hpp"
#include "render/backend_vulkan/vk_swapchain.hpp"

#include <array>

namespace eng
{

struct VulkanFramebuffer : public RFramebuffer
{
public:
    VulkanFramebuffer(
        VkDevice vk_device,
        const VulkanSwapchain*,
        VulkanTexture* depth_stencil,
        uint32_t imageIndex,
        const VulkanRenderPass*
    );
    VulkanFramebuffer(
        VkDevice vk_device,
        VulkanRenderPass*,
        VulkanTexture**,
        int num_textures,
        VulkanTexture *depth_stencil
    );
    ~VulkanFramebuffer() override;

    VkFramebuffer vk_framebuffer;
    VkDevice vk_device_ref;
};

}

#endif