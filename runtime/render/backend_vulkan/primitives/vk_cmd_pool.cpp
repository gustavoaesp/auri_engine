#include "backend_vulkan/primitives/vk_cmd_pool.hpp"
#include "backend_vulkan/primitives/vk_cmd_buffer.hpp"

namespace eng
{

VulkanCommandPool::VulkanCommandPool(VkDevice vk_device):
    vk_device_ref_(vk_device)
{
    VkCommandPoolCreateInfo vk_pool_info{};
    vk_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vk_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(vk_device, &vk_pool_info, nullptr, &vk_pool_) != VK_SUCCESS) {
        // TODO error
    }
}

VulkanCommandPool::~VulkanCommandPool()
{
    if (vk_pool_) {
        vkDestroyCommandPool(vk_device_ref_, vk_pool_, nullptr);
    }
    vk_pool_ = VK_NULL_HANDLE;
}

RCommandBuffer *VulkanCommandPool::CreateCommandBuffer(bool primary)
{
    VkCommandBufferAllocateInfo allocInfo{};
    VkCommandBuffer vk_buffer;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vk_pool_;
    allocInfo.level = (primary) ?
        VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(
        vk_device_ref_, &allocInfo, &vk_buffer) != VK_SUCCESS)
    {
        // TODO error
    }

    return new VulkanCommandBuffer(vk_buffer);
}

}