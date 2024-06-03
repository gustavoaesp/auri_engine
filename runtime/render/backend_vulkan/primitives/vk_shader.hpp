#ifndef _RENDER_BACKEND_VULKAN_SHADER_HPP_
#define _RENDER_BACKEND_VULKAN_SHADER_HPP_

#include <vulkan/vulkan.h>

#include "primitives/shader.hpp"

namespace eng
{

struct VulkanShader : public RShader
{
    VulkanShader(
        VkDevice vk_device,
        const char *filename,
        RShaderPipelineBind
    );
    ~VulkanShader() override;

    VkShaderModule vk_shader_module;
    VkDevice vk_device_ref;
};

}

#endif