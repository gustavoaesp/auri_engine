#include "backend_vulkan/primitives/vk_shader.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace eng
{

std::vector<uint8_t> ReadFile(const char *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::string err = "Could not open shader " + std::string(filename);
        std::cerr << err << "\n";
    }

    std::vector<uint8_t> buffer(file.tellg());
    file.seekg(0);

    file.read((char*)buffer.data(), buffer.size());

    return std::move(buffer);
}

VulkanShader::VulkanShader(
    VkDevice vk_device, const char *filename, RShaderPipelineBind pipeline_bind
):
    RShader(pipeline_bind),
    vk_device_ref(vk_device)
{
    std::vector<uint8_t> bytecode = ReadFile(filename);
    VkShaderModuleCreateInfo create_info{};

    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = bytecode.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

    if (vkCreateShaderModule(vk_device, &create_info, nullptr, &vk_shader_module) != VK_SUCCESS) {
        // TODO error
    }
}

VulkanShader::~VulkanShader()
{
    if (vk_shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(vk_device_ref, vk_shader_module, nullptr);
    }

    vk_shader_module = VK_NULL_HANDLE;
}

}