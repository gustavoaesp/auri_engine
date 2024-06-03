#include "backend_vulkan/synchronization/vk_fence.hpp"

#include <iostream>

namespace eng
{

VulkanFence::VulkanFence(VkDevice vk_device, bool signaled):
    vk_device_ref_(vk_device)
{
    VkFenceCreateInfo vk_fence_info{};
    vk_fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signaled) {
        vk_fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    if (vkCreateFence(vk_device, &vk_fence_info, nullptr, &vk_fence_) != VK_SUCCESS) {
        std::cerr << "Can't create fence\n";
    }
}

VulkanFence::~VulkanFence()
{
    if (vk_fence_ != VK_NULL_HANDLE) {
        vkDestroyFence(vk_device_ref_, vk_fence_, nullptr);
    }
    vk_fence_ = VK_NULL_HANDLE;
}

}
