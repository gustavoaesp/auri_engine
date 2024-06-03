#ifndef _RENDER_BACKEND_VULKAN_PIPELINE_HPP_
#define _RENDER_BACKEND_VULKAN_PIPELINE_HPP_

#include "primitives/pipeline.hpp"
#include "backend_vulkan/primitives/vk_render_pass.hpp"
#include "backend_vulkan/primitives/vk_shader.hpp"
#include "backend_vulkan/primitives/vk_descriptors.hpp"

#include <vulkan/vulkan.h>

namespace eng
{

struct VulkanVertexDescription
{
    std::array<VkVertexInputBindingDescription, 16> vertex_binding_descriptors;
    int vertex_binding_count;
    std::array<VkVertexInputAttributeDescription, 16> vertex_attributes_descriptors;
    int vertex_attribute_count;
};

struct VulkanPipeline : public RPipeline
{
    VulkanPipeline(
        VkDevice vk_device,
        const VulkanRenderPass *render_pass,
        const RBlendState *blend_state,
        const RDepthStencilState *depth_state,
        const VulkanShader **shaders,
        uint32_t num_shaders,
        const VulkanVertexDescription*,
        const VulkanDescriptorSetLayout**,
        int num_descriptor_set_layouts
    );
    ~VulkanPipeline() override;

    VkDevice vk_device_ref;
    VkPipelineLayout vk_pipeline_layout;
    VkPipeline vk_pipeline;
};

}

#endif