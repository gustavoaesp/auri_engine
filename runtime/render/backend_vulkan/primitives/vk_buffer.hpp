#ifndef _RENDER_BACKEND_VULKAN_BUFFER_HPP_
#define _RENDER_BACKEND_VULKAN_BUFFER_HPP_

#include <vulkan/vulkan.h>

#include "render/primitives/buffer.hpp"
#include "backend_vulkan/valloc.hpp"

#include "vk_mem_alloc.h"

namespace eng
{

struct VulkanBuffer : public RBuffer
{
    VulkanBuffer(
        VmaAllocator alloc,
        RBufferUsage usage,
        size_t size
    );
    ~VulkanBuffer() override;

    VmaAllocator vk_allocator_ref;
    VmaAllocation vk_allocation;
    VkBuffer vk_buffer;
};

}

#endif