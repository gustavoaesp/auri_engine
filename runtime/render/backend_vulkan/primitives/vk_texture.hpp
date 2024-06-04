#ifndef _BACKEND_VULKAN_TEXTURE_HPP_
#define _BACKEND_VULKAN_TEXTURE_HPP_

#include "render/primitives/texture.hpp"
#include "backend_vulkan/format_lookup.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>

namespace eng
{

struct VulkanTexture : public RTexture
{
    VulkanTexture(VkDevice, VmaAllocator, EFormat fmt, int width, int height);
    ~VulkanTexture() override;

    VkImage vk_image;
    VkImageView vk_image_view;
    VkFormat vk_fmt;

    VmaAllocation vk_allocation;
    VkDevice vk_device_ref;
    VmaAllocator vk_allocator_ref;
};

}

#endif