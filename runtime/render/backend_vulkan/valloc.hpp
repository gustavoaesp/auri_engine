#ifndef _RENDER_BACKEND_VULKAN_VALLOC_HPP_
#define _RENDER_BACKEND_VULKAN_VALLOC_HPP_

#include <vulkan/vulkan.h>
#include "backend_vulkan/vk_device.hpp"

namespace eng
{

enum class EMemoryLocation
{
    kMemoryLocationHost,
    kMemoryLocationDeviceHostVisible,
    kMemoryLocationDevice
};

enum class EMemoryType {
    kMemoryDiscrete,
    kMemoryIntegrated
};

class VAlloc
{
public:
    VAlloc(
        VulkanDevice* device,
        const VkPhysicalDeviceMemoryProperties&
    );
    ~VAlloc();

    void AllocateDeviceMemory(VkDeviceMemory* pDevMemory,
                                const VkMemoryRequirements& memRequirements,
                                EMemoryLocation location);

    void AllocateBuffer(VkBuffer, VkDeviceMemory*, EMemoryLocation);
    void AllocateTextureImage(VkImage, VkDeviceMemory*, EMemoryLocation);
private:
    VkPhysicalDeviceMemoryProperties properties_;
    EMemoryType memType_;

    VulkanDevice *device_;
};

}

#endif