#ifndef _RENDER_BACKEND_VULKAN_SAMPLER_HPP_
#define _RENDER_BACKEND_VULKAN_SAMPLER_HPP_

#include <vulkan/vulkan.h>

#include "primitives/sampler.hpp"

namespace eng
{

struct VulkanSampler : public RSampler
{
    VulkanSampler(VkDevice, const RSamplerAttributes*);
    ~VulkanSampler() override;

    VkSampler vk_sampler;
    VkDevice vk_device_ref;
};

}

#endif