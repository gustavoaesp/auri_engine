#ifndef _RENDER_BACKEND_VULKAN_INSTANCE_HPP_
#define _RENDER_BACKEND_VULKAN_INSTANCE_HPP_

#include <vulkan/vulkan.h>

#include <vector>

namespace eng
{

class VulkanInstance
{
public:
    VulkanInstance(const std::vector<const char*>& enabledExtensions,
                    const std::vector<const char*>& enabledLayers);
    VulkanInstance(VulkanInstance&&);
    VulkanInstance& operator=(VulkanInstance&&);

    ~VulkanInstance();

    static std::vector<VkExtensionProperties> GetAvailableExtensions();
    static std::vector<VkLayerProperties> GetValidationLayers();

    VkInstance get() const { return instance_; }
private:
    VkInstance instance_;
};

}

#endif