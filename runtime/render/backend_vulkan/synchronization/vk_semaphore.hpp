#ifndef _RENDER_BACKEND_VULKAN_SEMAPHORE_HPP_
#define _RENDER_BACKEND_VULKAN_SEMAPHORE_HPP_

#include <vulkan/vulkan.h>

namespace eng
{

class VulkanSemaphore
{
public:
    VulkanSemaphore(VkDevice);
    ~VulkanSemaphore();

    VkSemaphore GetVkHandle() { return vk_sem_; }
private:
    VkSemaphore vk_sem_;
    VkDevice vk_device_ref_;
};

}

#endif