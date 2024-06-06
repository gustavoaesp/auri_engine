#ifndef _BACKEND_VULKAN_DESCRIPTORS_HPP_
#define _BACKEND_VULKAN_DESCRIPTORS_HPP_

#include <vulkan/vulkan.h>

#include "primitives/descriptors.hpp"

namespace eng
{

struct VulkanDescriptorSetLayout : public RDescriptorLayout
{
    VulkanDescriptorSetLayout(
        VkDevice vk_device,
        const RDescriptorLayoutBinding *,
        int num_bindings
    );

    ~VulkanDescriptorSetLayout() override;

    VkDescriptorSetLayout vk_descriptor_set_layout;
    VkDevice vk_device_ref;
};

struct VulkanDescriptorPool : public RDescriptorPool
{
    VulkanDescriptorPool(
        VkDevice vk_device,
        size_t max_sets,
        RDescriptorLayoutBindingType
        );
    virtual ~VulkanDescriptorPool();

    VkDescriptorSet Allocate(VulkanDescriptorSetLayout*);

    RDescriptorSet *AllocateSet(RDescriptorLayout *) override;

    VkDescriptorPool vk_descriptor_pool;
    VkDevice vk_device_ref;
};

struct VulkanDescriptorSet : public RDescriptorSet
{
    VulkanDescriptorSet(VkDevice, VkDescriptorSet);
    virtual ~VulkanDescriptorSet();

    virtual void BindBuffers(
        uint32_t start_index,
        const RBufferBinding*,
        uint32_t count
    ) override;

    virtual void BindTextures(
        uint32_t start_index,
        const RTextureSamplerBinding*,
        uint32_t count
    ) override;

    VkDescriptorSet vk_descriptor_set;
    VkDevice vk_device_ref;
};

}

#endif