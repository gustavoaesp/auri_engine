#ifndef _BACKEND_VULKAN_VTX_INPUT_DESC_HPP_
#define _BACKEND_VULKAN_VTX_INPUT_DESC_HPP_
#include "backend_vulkan/primitives/vk_pipeline.hpp"

#include <vector>

#include <vulkan/vulkan.h>

#include "vertex.hpp"

namespace eng
{

VulkanVertexDescription VulkanBuildDescriptors(RVertexType vertex_type, RVertexType instance_type);

}

#endif