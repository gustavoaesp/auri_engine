#include "backend_vulkan/primitives/vk_descriptors.hpp"
#include "backend_vulkan/primitives/vk_sampler.hpp"
#include "backend_vulkan/primitives/vk_texture.hpp"

#include <vector>
#include "vk_descriptors.hpp"

namespace eng
{

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
    VkDevice vk_device, const RDescriptorLayoutBinding *bindings, int num_bindings):
    vk_device_ref(vk_device)
{
    VkDescriptorSetLayoutCreateInfo vk_layout_create;
    std::vector<VkDescriptorSetLayoutBinding> vk_layout_bindings;

    vk_layout_bindings.reserve(num_bindings);

    for (int i = 0; i < num_bindings; ++i) {
        VkShaderStageFlags vk_stage_flags = 0;
        vk_layout_bindings.push_back({});
        auto& binding = vk_layout_bindings.back();

        binding.binding = bindings[i].bindingIndex;

        switch(bindings[i].type) {
        case RDescriptorLayoutBindingType::kTextureSampler:
            binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        case RDescriptorLayoutBindingType::kUniformBuffer:
            binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        }

        if (bindings[i].bindingStageAccessFlags & EShaderStageVertexBit) {
            vk_stage_flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (bindings[i].bindingStageAccessFlags & EShaderStageFragmentBit) {
            vk_stage_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        binding.stageFlags = vk_stage_flags;
        binding.descriptorCount = 1;
        binding.pImmutableSamplers = nullptr;
    }

    vk_layout_create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vk_layout_create.bindingCount = num_bindings;
    vk_layout_create.pBindings = vk_layout_bindings.data();

    if (vkCreateDescriptorSetLayout(
        vk_device,
        &vk_layout_create,
        nullptr,
        &vk_descriptor_set_layout
    ) != VK_SUCCESS) {
        // TODO error
    }
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    if (vk_descriptor_set_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(vk_device_ref, vk_descriptor_set_layout, nullptr);
    }

    vk_descriptor_set_layout = VK_NULL_HANDLE;
}

/* ======================================================================
*
*   DESCRIPTOR POOL
*
=======================================================================*/

VulkanDescriptorPool::VulkanDescriptorPool(
    VkDevice vk_device,
    size_t max_sets,
    RDescriptorLayoutBindingType binding_type
):
    vk_device_ref(vk_device)
{
    VkDescriptorPoolSize pool_size{};
    VkDescriptorPoolCreateInfo pool_info{};

    switch(binding_type) {
    case RDescriptorLayoutBindingType::kTextureSampler:
        pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
    case RDescriptorLayoutBindingType::kUniformBuffer:
        pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
    }
    pool_size.descriptorCount = max_sets;

    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = max_sets;

    if(vkCreateDescriptorPool(vk_device, &pool_info, nullptr, &vk_descriptor_pool) != VK_SUCCESS) {
        // TODO error
    }
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
    if (vk_descriptor_pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(vk_device_ref, vk_descriptor_pool, nullptr);
    }

    vk_descriptor_pool = VK_NULL_HANDLE;
}

VkDescriptorSet VulkanDescriptorPool::Allocate(VulkanDescriptorSetLayout *desc_layout)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vk_descriptor_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &desc_layout->vk_descriptor_set_layout;

    VkDescriptorSet vk_descriptor_set;
    if (vkAllocateDescriptorSets(vk_device_ref, &allocInfo, &vk_descriptor_set) != VK_SUCCESS) {
        // TODO error
    }

    return vk_descriptor_set;
}

/* ======================================================================
 *
 *   DESCRIPTOR SET
 *
 =======================================================================*/

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice vk_device, VkDescriptorSet set):
    vk_descriptor_set(set), vk_device_ref(vk_device)
{}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
}

void VulkanDescriptorSet::BindBuffers(
    uint32_t start_index,
    const RBufferBinding* bindings,
    uint32_t count)
{
    /*std::array<VkDescriptorBufferInfo, kMaxDescriptorBindings> bufferInfos{};

    for (int i = 0; i < count; i++) {
        bufferInfos[i].buffer = bindings[i].buffer
    }*/
}

void VulkanDescriptorSet::BindTextures(
    uint32_t start_index,
    const RTextureSamplerBinding* bindings,
    uint32_t count)
{
    std::array<VkWriteDescriptorSet, kMaxDescriptorBindings> descriptor_writes{};
    std::array<VkDescriptorImageInfo, kMaxDescriptorBindings> image_infos{};

    for (int i = 0; i < count; ++i) {
        VulkanTexture *vulkan_texture = static_cast<VulkanTexture*>(bindings[i].texture);
        VulkanSampler *vulkan_sampler = static_cast<VulkanSampler*>(bindings[i].sampler);

        image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_infos[i].imageView = vulkan_texture->vk_image_view;
        image_infos[i].sampler = vulkan_sampler->vk_sampler;

        descriptor_writes[i].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[i].pImageInfo = &image_infos[i];
        descriptor_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[i].dstSet = vk_descriptor_set;
        descriptor_writes[i].dstBinding = start_index + i;
        descriptor_writes[i].descriptorCount = 1;
    }

    vkUpdateDescriptorSets(
        vk_device_ref,
        count,
        descriptor_writes.data(),
        0, nullptr
    );
}

}