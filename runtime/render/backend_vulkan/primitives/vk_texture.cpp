#include "backend_vulkan/primitives/vk_texture.hpp"

namespace eng
{

VulkanTexture::VulkanTexture(VkDevice vk_device, VmaAllocator vma_allocator, EFormat fmt, int width, int height):
    vk_device_ref(vk_device), vk_allocator_ref(vma_allocator), RTexture(width, height, fmt)
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
    vmaDestroyImage(vk_allocator_ref, vk_image, vk_allocation);
}

}