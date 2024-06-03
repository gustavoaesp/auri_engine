#pragma once

//#include "vulkan/swapchain.hpp"
#include "backend_vulkan/primitives/vk_texture.hpp"
#include "render/primitives/render_pass.hpp"

#include <vector>

namespace eng
{

class VulkanSwapchain;

struct VulkanRenderPass : public RRenderPass
{
    VulkanRenderPass() {}
    VulkanRenderPass(VkDevice vk_device, const VulkanSwapchain*, bool depth_test);
    VulkanRenderPass(
        VkDevice vk_device,
        const RRenderPassAttachment *color_attachments,
        int num_attachments,
        const RRenderPassAttachment *depth_attachment
    );
    ~VulkanRenderPass() override;

    VkRenderPass vk_render_pass;
    VkDevice vk_device_ref;
};

}