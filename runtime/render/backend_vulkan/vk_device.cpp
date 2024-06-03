#include "backend_vulkan/vk_device.hpp"
#include "backend_vulkan/vk_swapchain.hpp"

#include <array>
#include <optional>

namespace eng
{

std::array<const char*, 1> g_requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanDevice::VulkanDevice(const VulkanInstance& vulkanInstance, VkSurfaceKHR windowSurface)
{
    PickPhysicalDevice(vulkanInstance);
    SetQueueFamilyIndices(vulkanInstance, windowSurface);

    std::array<uint32_t, 2> queueFamilyIndices = {
        queueFamilies_.graphicsFamilyIndex,
        queueFamilies_.presentationFamilyIndex
    };

    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t queueFamily : queueFamilyIndices) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    VkPhysicalDeviceFeatures deviceFeatures{}; // Off by now
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(g_requiredExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = g_requiredExtensions.data();

    if (vkCreateDevice(physical_, &deviceCreateInfo, nullptr, &device_) != VK_SUCCESS) {
        // TODO error
    }

    vkGetDeviceQueue(device_, queueFamilies_.graphicsFamilyIndex, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, queueFamilies_.presentationFamilyIndex, 0, &presentationQueue_);
}

VulkanDevice::~VulkanDevice()
{
    vkDestroyDevice(device_, nullptr);
}

void VulkanDevice::GraphicsSubmit(VulkanCommandBuffer* buffer,
                                    VulkanSemaphore* sem_wait,
                                    VulkanSemaphore* sem_signal,
                                    EStageWait stage_wait,
                                    VulkanFence* fence)
{
    std::array<VkCommandBuffer, 1> vk_buffer_array = { buffer->vk_command_buffer };

    GraphicsSubmitInternal(vk_buffer_array.data(), 1, sem_wait, sem_signal, stage_wait, fence);
}

void VulkanDevice::PresentSubmit(VulkanSwapchain& swapchain,
    VulkanSemaphore* sem_wait, uint32_t* imageIndex)
{
    VkPresentInfoKHR presentInfo{};
    VkSemaphore vk_sem = sem_wait->GetVkHandle();
    std::array<VkSwapchainKHR, 1> swapchain_array = { swapchain.get() };

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    if (sem_wait) {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &vk_sem;
    }

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchain_array.data();
    presentInfo.pImageIndices = imageIndex;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(presentationQueue_, &presentInfo);
}

void VulkanDevice::WaitIdle()
{
    vkDeviceWaitIdle(device_);
}

void VulkanDevice::PickPhysicalDevice(const VulkanInstance& vulkanInstance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanInstance.get(), &deviceCount, nullptr);
    if (!deviceCount) {
        // TODO error
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanInstance.get(), &deviceCount, devices.data());
    for (const VkPhysicalDevice& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physical_ = device;
        }
    }

    vkGetPhysicalDeviceMemoryProperties(physical_, &memProperties_);
}

void VulkanDevice::SetQueueFamilyIndices(const VulkanInstance& vulkanInstance, VkSurfaceKHR windowSurface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_, &queueFamilyCount, queueFamilies.data());

    std::optional<uint32_t> graphicsFamilyIndex;
    std::optional<uint32_t> presentationFamilyIndex;

    uint32_t index = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamilyIndex = index;
        }
        VkBool32 presentationSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            physical_,
            index,
            windowSurface,
            &presentationSupport
        );
        if (presentationSupport) {
            presentationFamilyIndex = index;
        }
        ++index;
    }

    if (!graphicsFamilyIndex.has_value()) {
        //ExitError("No graphics family queue supported");
        // TODO error
    }
    if (!presentationFamilyIndex.has_value()) {
        //ExitError("No presentation family queue supported");
        // TODO error
    }

    queueFamilies_.graphicsFamilyIndex = graphicsFamilyIndex.value();
    queueFamilies_.presentationFamilyIndex = presentationFamilyIndex.value();

}

void VulkanDevice::GraphicsSubmitInternal(
    VkCommandBuffer* buffers, uint32_t nBuffers,
    VulkanSemaphore* sem_wait, VulkanSemaphore *sem_signal,
    EStageWait stage_wait,
    VulkanFence* fence
)
{
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags vk_wait_stage;

        std::array<VkSemaphore, 1> sem_wait_array = {
            (sem_wait) ? sem_wait->GetVkHandle() : VK_NULL_HANDLE
        };
        std::array<VkSemaphore, 1> sem_signal_array = {
            (sem_signal) ? sem_signal->GetVkHandle() : VK_NULL_HANDLE
        };

        switch(stage_wait) {
        case EStageWait::kStageColorAttachment:
            vk_wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
            //ExitError("Semaphore pipeline wait stage invalid");
            // TODO error
        }

        if (sem_wait) {
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = sem_wait_array.data();
            submitInfo.pWaitDstStageMask = &vk_wait_stage;
        }

        if (sem_signal) {
            submitInfo.pSignalSemaphores = sem_signal_array.data();
            submitInfo.signalSemaphoreCount = 1;
        }

        submitInfo.commandBufferCount = nBuffers;
        submitInfo.pCommandBuffers = buffers;

        if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo,
                (fence) ? fence->GetVkHandle() : VK_NULL_HANDLE) != VK_SUCCESS)
        {
            //ExitError("Error submiting graphics queue");
            // TODO error
        }
}

void VulkanDevice::GraphicsSubmit(
    VulkanSemaphore* sem_wait, VulkanSemaphore* sem_signal,
    EStageWait stage_wait, VulkanFence* fence,
    VkCommandBuffer* vk_buffers, uint32_t num_buffers)
{
    GraphicsSubmitInternal(vk_buffers, num_buffers, sem_wait, sem_signal, stage_wait, fence);
}

}