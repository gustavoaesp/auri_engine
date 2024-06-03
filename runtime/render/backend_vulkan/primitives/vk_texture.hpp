#ifndef _BACKEND_VULKAN_TEXTURE_HPP_
#define _BACKEND_VULKAN_TEXTURE_HPP_

#include "render/primitives/texture.hpp"
#include "backend_vulkan/format_lookup.hpp"

#include <vulkan/vulkan.h>

namespace eng
{

struct VulkanTexture : public RTexture
{
    VulkanTexture(VkDevice, EFormat fmt, int width, int height);
    ~VulkanTexture() override;

    VkImage vk_image;
    VkDeviceMemory vk_dev_memory;
    VkImageView vk_image_view;
    VkFormat vk_fmt;

    VkDevice vk_device_ref;
};

}

#endif