#include "backend_vulkan/valloc.hpp"

namespace eng
{

void VAlloc::AllocateDeviceMemory(
    VkDeviceMemory* pDevMemory,
    const VkMemoryRequirements& memRequirements,
    EMemoryLocation location
)
{
    VkMemoryPropertyFlags property_flags = 0;
    VkMemoryPropertyFlags property_not_active = 0;

    if (memType_ == EMemoryType::kMemoryIntegrated) {
        location = EMemoryLocation::kMemoryLocationDeviceHostVisible;
    }

    for (uint32_t i = 0; i < properties_.memoryTypeCount; ++i) {
        uint32_t bit = (1 << i);
        if (!(bit & memRequirements.memoryTypeBits)) { continue; }

        switch (location)
        {
        case EMemoryLocation::kMemoryLocationDevice:
            property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            property_not_active = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        case EMemoryLocation::kMemoryLocationHost:
            property_flags =
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        case EMemoryLocation::kMemoryLocationDeviceHostVisible:
            property_flags =
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        }

        if ((property_flags & properties_.memoryTypes[i].propertyFlags) != property_flags
            && !(property_not_active & properties_.memoryTypes[i].propertyFlags)) {
            continue;
        }

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = i;

        if (vkAllocateMemory(device_->get(), &allocInfo, nullptr, pDevMemory) != VK_SUCCESS) {
            continue;
        }

        return;
    }

    //ExitError("Could not allocate memory for vtx buffer");
    // TODO error
}

VAlloc::VAlloc(VulkanDevice* device, const VkPhysicalDeviceMemoryProperties& props):
    device_(device), properties_(props), memType_(EMemoryType::kMemoryDiscrete)
{
}

VAlloc::~VAlloc()
{
}

void VAlloc::AllocateBuffer(VkBuffer buffer, VkDeviceMemory* pDevMemory, EMemoryLocation location)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_->get(), buffer, &memRequirements);

    AllocateDeviceMemory(pDevMemory, memRequirements, location);
}

void VAlloc::AllocateTextureImage(VkImage image, VkDeviceMemory* pDevMemory, EMemoryLocation location)
{
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_->get(), image, &memRequirements);

    AllocateDeviceMemory(pDevMemory, memRequirements, location);
}


}