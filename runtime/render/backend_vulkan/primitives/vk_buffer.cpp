#include "backend_vulkan/primitives/vk_buffer.hpp"
#include "vk_buffer.hpp"

namespace eng
{

VulkanBuffer::VulkanBuffer(VkDevice vk_device, VAlloc* vk_alloc, RBufferUsage usage, size_t size):
    vk_device_ref(vk_device), RBuffer(usage)
{
    VkBufferCreateInfo vk_buffer_info{};
    vk_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vk_buffer_info.size = size;
    switch (usage) {
    case RBufferUsage::kIndex:
        vk_buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case RBufferUsage::kVertex:
        vk_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case RBufferUsage::kUniform:
        vk_buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    }
    vk_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vk_device, &vk_buffer_info, nullptr, &vk_buffer) != VK_SUCCESS) {
        // TODO error
    }

    vk_alloc->AllocateBuffer(
        vk_buffer,
        &vk_dev_mem,
        EMemoryLocation::kMemoryLocationDeviceHostVisible
    );

    vkBindBufferMemory(vk_device, vk_buffer, vk_dev_mem, 0);
}

VulkanBuffer::~VulkanBuffer()
{
    if (vk_buffer) {
        vkDestroyBuffer(vk_device_ref, vk_buffer, nullptr);
        vkFreeMemory(vk_device_ref, vk_dev_mem, nullptr);
    }
}

}