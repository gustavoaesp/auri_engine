#include "backend_vulkan/synchronization/vk_semaphore.hpp"

#include <iostream>

namespace eng
{

VulkanSemaphore::VulkanSemaphore(VkDevice vk_device):
    vk_device_ref_(vk_device)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(vk_device, &semaphoreInfo, nullptr, &vk_sem_) != VK_SUCCESS) {
        std::cerr << "Failed to create sempahore\n";
    }
}

VulkanSemaphore::~VulkanSemaphore()
{
    if (vk_sem_ != VK_NULL_HANDLE) {
        vkDestroySemaphore(vk_device_ref_, vk_sem_, nullptr);
    }

    vk_sem_ = VK_NULL_HANDLE;
}

}
