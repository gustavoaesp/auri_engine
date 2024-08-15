#include "backend_vulkan/vertex_input_desc.hpp"

namespace eng
{

VulkanVertexDescription VulkanBuildDescriptors(RVertexType vertex_type, RVertexType instance_type)
{
    VulkanVertexDescription desc_out{};
    VkVertexInputAttributeDescription *output = desc_out.vertex_attributes_descriptors.data();

    desc_out.vertex_binding_count = 1;

    switch (vertex_type)
    {
    case RVertexType::kVertexPos3Col4:
        desc_out.vertex_attribute_count = 2;

        output[0].binding = 0;
        output[0].location = 0;
        output[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        output[0].offset = 0;

        output[1].binding = 0;
        output[1].location = 1;
        output[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        output[1].offset = sizeof(float) * 3;

        desc_out.vertex_binding_descriptors[0].binding = 0;
        desc_out.vertex_binding_descriptors[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        desc_out.vertex_binding_descriptors[0].stride = sizeof(Vertex);

        break;
    case RVertexType::kVertexPos3Nor3Tex2:
        desc_out.vertex_attribute_count = 3;

        output[0].binding = 0;
        output[0].location = 0;
        output[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        output[0].offset = 0;

        output[1].binding = 0;
        output[1].location = 1;
        output[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        output[1].offset = sizeof(float) * 3;

        output[2].binding = 0;
        output[2].location = 2;
        output[2].format = VK_FORMAT_R32G32_SFLOAT;
        output[2].offset = sizeof(float) * 6;

        desc_out.vertex_binding_descriptors[0].binding = 0;
        desc_out.vertex_binding_descriptors[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        desc_out.vertex_binding_descriptors[0].stride = sizeof(Vertex_NorTuv);
        break;
    case RVertexType::kVertexPos3Nor3Tex2_Skinned:
        desc_out.vertex_attribute_count = 5;
 
        output[0].binding = 0;
        output[0].location = 0;
        output[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        output[0].offset = 0;

        output[1].binding = 0;
        output[1].location = 1;
        output[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        output[1].offset = sizeof(float) * 3;

        output[2].binding = 0;
        output[2].location = 2;
        output[2].format = VK_FORMAT_R32G32_SFLOAT;
        output[2].offset = sizeof(float) * 6;

        output[3].binding = 0;
        output[3].location = 3;
        output[3].format = VK_FORMAT_R32G32B32A32_UINT;
        output[3].offset = sizeof(float) * 8;

        output[4].binding = 0;
        output[4].location = 4;
        output[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        output[4].offset = (sizeof(float) * 8) + (sizeof(uint32_t) * 4);

        desc_out.vertex_binding_descriptors[0].binding = 0;
        desc_out.vertex_binding_descriptors[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        desc_out.vertex_binding_descriptors[0].stride = sizeof(Vertex_NorTuv_Skinned);
        break;
    }

    return desc_out;
}

}