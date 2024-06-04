#include "backend_vulkan/primitives/vk_buffer.hpp"
#include "vk_buffer.hpp"

namespace eng
{

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, RBufferUsage usage, size_t size):
    vk_allocator_ref(allocator), RBuffer(usage)
{
    VkBufferCreateInfo vk_buffer_info{};
    VmaAllocationCreateInfo alloc_create_info{};

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


    //alloc_create_info.memoryTypeBits = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateBuffer(
        allocator,
        &vk_buffer_info,
        &alloc_create_info,
        &vk_buffer,
        &vk_allocation,
        nullptr
    );
}

VulkanBuffer::~VulkanBuffer()
{
    vmaDestroyBuffer(vk_allocator_ref, vk_buffer, vk_allocation);
}

}