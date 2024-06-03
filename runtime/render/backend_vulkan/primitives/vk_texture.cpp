#include "backend_vulkan/primitives/vk_texture.hpp"

namespace eng
{

VulkanTexture::VulkanTexture(VkDevice vk_device, EFormat fmt, int width, int height):
    vk_device_ref(vk_device), RTexture(width, height, fmt)
{
    const auto& fmt_map_pair = fmt_map.find(fmt);
    if (fmt_map_pair == fmt_map.end()) {
        // TODO error
        return;
    }
    vk_fmt = fmt_map_pair->second;
}

VulkanTexture::~VulkanTexture()
{
    vkDestroyImageView(vk_device_ref, vk_image_view, nullptr);
    vkDestroyImage(vk_device_ref, vk_image, nullptr);
    vkFreeMemory(vk_device_ref, vk_dev_memory, nullptr);
}

}