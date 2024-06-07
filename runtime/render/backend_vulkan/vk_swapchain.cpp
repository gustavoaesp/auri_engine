#include "backend_vulkan/vk_swapchain.hpp"
#include "backend_vulkan/vk_device.hpp"

#include <array>

namespace eng
{

VulkanSwapchain::VulkanSwapchain(
    VulkanInstance& instance, VulkanDevice* device,
    GLFWwindow* window, VkSurfaceKHR windowSurface
):
    instance_(&instance), windowSurface_(windowSurface), device_(device),
    window_(window)
{
    int winWidth, winHeight;
    glfwGetFramebufferSize(window, &winWidth, &winHeight);

    std::vector<VkSurfaceFormatKHR> formats;

    VkSurfaceCapabilitiesKHR capabilities;
    VkExtent2D extent = {
        static_cast<uint32_t>(winWidth),
        static_cast<uint32_t>(winHeight)
    };

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysical(), windowSurface_,
                                        &formatCount, nullptr);
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysical(), windowSurface_,
                                        &formatCount, formats.data());
    VkSurfaceFormatKHR selectedFormat;
    for (const VkSurfaceFormatKHR& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            selectedFormat = format;
            break;
        }
    }
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->getPhysical(), windowSurface_, &capabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = windowSurface_;
    createInfo.minImageCount = capabilities.minImageCount + 1;
    createInfo.imageFormat = selectedFormat.format;
    createInfo.imageColorSpace = selectedFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VulkanQueueFamilyIndices& queueFamilies = *device->getQueueFamilies();


    if (queueFamilies.graphicsFamilyIndex != queueFamilies.presentationFamilyIndex) {
        std::array<uint32_t, 2> queueFamiliesSet = {
            queueFamilies.presentationFamilyIndex,
            queueFamilies.graphicsFamilyIndex
        };

        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamiliesSet.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    extent_ = extent;
    format_ = selectedFormat.format;

    if (vkCreateSwapchainKHR(device->get(), &createInfo, nullptr, &swapchain_) != VK_SUCCESS) {
        //ExitError("Can't create swap chain object");
        // TODO error
    }

    CreateImages();
}

VulkanSwapchain::~VulkanSwapchain()
{
    for (auto imageView : imageViews_) {
        vkDestroyImageView(device_->get(), imageView, nullptr);
    }
    vkDestroySwapchainKHR(device_->get(), swapchain_, nullptr);
    vkDestroySurfaceKHR(instance_->get(), windowSurface_, nullptr);
}

void VulkanSwapchain::CreateImages()
{
    uint32_t swapChainImageCount = 0;
    vkGetSwapchainImagesKHR(
        device_->get(),
        swapchain_,
        &swapChainImageCount,
        nullptr
    );
    images_.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(
        device_->get(),
        swapchain_,
        &swapChainImageCount,
        images_.data()
    );

    imageViews_.resize(swapChainImageCount);
    for (size_t i = 0; i < images_.size(); ++i) {
        VkImage image = images_[i];
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format_;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(
                device_->get(),
                &createInfo,
                nullptr,
                &imageViews_[i]) != VK_SUCCESS)
        {
            //ExitError("Can't create image view");
            // TODO error
        }
    }
}


}