#include "backend_vulkan/instance.hpp"

#include "GLFW/glfw3.h"

namespace eng
{

VulkanInstance::VulkanInstance(const std::vector<const char*>& enabledExtensions,
                                const std::vector<const char*>& enabledLayers)
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    createInfo.enabledLayerCount = enabledLayers.size();
    if (enabledLayers.size()) {
        createInfo.ppEnabledLayerNames = enabledLayers.data();
    }

    vkCreateInstance(&createInfo, nullptr, &instance_);
}

VulkanInstance::VulkanInstance(VulkanInstance&& rhs):
    instance_(rhs.instance_)
{
    rhs.instance_ = VK_NULL_HANDLE;
}

VulkanInstance& VulkanInstance::operator=(VulkanInstance&& rhs)
{
    instance_ = rhs.instance_;
    rhs.instance_ = VK_NULL_HANDLE; 
    return *this;
}

VulkanInstance::~VulkanInstance()
{
    if (!instance_) {
        return;
    }
    vkDestroyInstance(instance_, nullptr);
}

std::vector<VkExtensionProperties> VulkanInstance::GetAvailableExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    auto extensionProperties = std::vector<VkExtensionProperties>(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

    return extensionProperties;
}

std::vector<VkLayerProperties> VulkanInstance::GetValidationLayers()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return availableLayers;
}

}