#include "backend_vulkan/primitives/vk_sampler.hpp"

namespace eng
{

VulkanSampler::VulkanSampler(VkDevice vk_device, const RSamplerAttributes* attributes):
    vk_device_ref(vk_device)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    VkFilter filter;
    switch (attributes->filter_mode) {
    case RSamplerFilterMode::kFilterLinear:
        filter = VK_FILTER_LINEAR;
        break;
    case RSamplerFilterMode::kFilterNearest:
        filter = VK_FILTER_NEAREST;
        break;
    }

    samplerInfo.magFilter = samplerInfo.minFilter = filter;

    VkSamplerAddressMode nativeAddressMode;
    switch (attributes->address_mode) {
    case RSamplerAddressMode::kRepeat:
        nativeAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        break;
    case RSamplerAddressMode::kMirroredRepeat:
        nativeAddressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        break;
    case RSamplerAddressMode::kClamp:
        nativeAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        break;
    }

    // Set all to the same value
    samplerInfo.addressModeU = samplerInfo.addressModeV =
        samplerInfo.addressModeW = nativeAddressMode;

    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 8;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;

    samplerInfo.maxLod = 16;

    if (vkCreateSampler(vk_device, &samplerInfo, nullptr, &vk_sampler) != VK_SUCCESS) {
        // TODO error
    }
}

VulkanSampler::~VulkanSampler()
{
    if (vk_sampler != VK_NULL_HANDLE) {
        vkDestroySampler(vk_device_ref, vk_sampler, nullptr);
    }

    vk_sampler = VK_NULL_HANDLE;
}

}