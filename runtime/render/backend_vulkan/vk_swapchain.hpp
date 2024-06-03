#ifndef _RENDER_BACKEND_VULKAN_SWAPCHAIN_HPP_
#define _RENDER_BACKEND_VULKAN_SWAPCHAIN_HPP_

#include <GLFW/glfw3.h>
#include "backend_vulkan/instance.hpp"
#include "backend_vulkan/vk_device.hpp"

namespace eng
{

class VulkanSwapchain
{
public:
    VulkanSwapchain() {}
    VulkanSwapchain(VulkanInstance& instance, VulkanDevice*, GLFWwindow*, VkSurfaceKHR);
    ~VulkanSwapchain();

    VkSwapchainKHR& get() { return swapchain_; }

    const VkExtent2D& getExtent() const { return extent_; }
    const VkFormat& getFormat() const { return format_; }
    VkImage getImage(size_t index) const { return images_[index]; }
    VkImageView getImageView(size_t index) const { return imageViews_[index]; }

    uint32_t getImageCount() { return images_.size(); }

private:
    VulkanInstance* instance_;
    VkSwapchainKHR swapchain_;
    VkSurfaceKHR windowSurface_;
    std::vector<VkImage> images_;
    std::vector<VkImageView> imageViews_;

    VkFormat format_;
    VkExtent2D extent_;

    void CreateImages();

    VulkanDevice *device_;
};

}

#endif