#ifndef _RENDER_BACKEND_VULKAN_BUFFER_HPP_
#define _RENDER_BACKEND_VULKAN_BUFFER_HPP_

#include <vulkan/vulkan.h>

#include "render/primitives/buffer.hpp"
#include "backend_vulkan/valloc.hpp"

namespace eng
{

struct VulkanBuffer : public RBuffer
{
    VulkanBuffer(
        VkDevice vk_device,
        VAlloc *vk_alloc,
        RBufferUsage usage,
        size_t size
    );
    ~VulkanBuffer() override;

    VkDeviceMemory vk_dev_mem;
    VkBuffer vk_buffer;
    VkDevice vk_device_ref;
};

}

#endif