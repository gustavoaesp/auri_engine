#ifndef _RENDER_BACKEND_VULKAN_FENCE_HPP_
#define _RENDER_BACKEND_VULKAN_FENCE_HPP_

#include <vulkan/vulkan.h>

namespace eng
{

class VulkanFence
{
public:
    VulkanFence(VkDevice, bool signaled);
    ~VulkanFence();

    VkFence GetVkHandle() { return vk_fence_; }
private:
    VkFence vk_fence_;
    VkDevice vk_device_ref_;
};

}

#endif