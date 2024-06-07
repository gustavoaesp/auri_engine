#ifndef _RENDER_BACKEND_VULKAN_DEVICE_HPP_
#define _RENDER_BACKEND_VULKAN_DEVICE_HPP_

#include <vulkan/vulkan.h>

#include "backend_vulkan/instance.hpp"
#include "backend_vulkan/synchronization/vk_semaphore.hpp"
#include "backend_vulkan/synchronization/vk_fence.hpp"
#include "backend_vulkan/primitives/vk_cmd_buffer.hpp"
//#include "backend_vulkan/vk_swapchain.hpp"

namespace eng
{

class VulkanSwapchain;

enum class EStageWait
{
    kStageColorAttachment
};

struct VulkanQueueFamilyIndices
{
    uint32_t graphicsFamilyIndex;
    uint32_t presentationFamilyIndex;
};

class VulkanDevice
{
public:
    VulkanDevice(const VulkanInstance&, VkSurfaceKHR);
    ~VulkanDevice();

    VkDevice get() const { return device_; };
    VkPhysicalDevice getPhysical() const { return physical_; }

    VulkanQueueFamilyIndices* getQueueFamilies() { return &queueFamilies_; }

    void GraphicsSubmit(VulkanCommandBuffer* buffer,
                        VulkanSemaphore* sem_wait,
                        VulkanSemaphore* sem_signal,
                        EStageWait stage_wait,
                        VulkanFence* fence);

    void GraphicsSubmit(VulkanSemaphore* sem_wait, VulkanSemaphore* sem_signal,
                        EStageWait stage_wait, VulkanFence* fence,
                        VkCommandBuffer*, uint32_t num_buffers);

    void PresentSubmit(
        VulkanSwapchain& swapchain,
        VulkanSemaphore* sem_wait,
        uint32_t* imageIndex
    );

    void WaitIdle();

    VkPhysicalDeviceMemoryProperties& GetMemoryProperties() { return memProperties_; }

    VkQueue GetQueue() const { return graphicsQueue_; }
private:
    VkPhysicalDevice physical_;
    VkDevice device_;

    VulkanQueueFamilyIndices queueFamilies_;
    VkQueue graphicsQueue_;
    VkQueue presentationQueue_;

    VkPhysicalDeviceMemoryProperties memProperties_;
    void PickPhysicalDevice(const VulkanInstance&);
    void SetQueueFamilyIndices(const VulkanInstance&, VkSurfaceKHR);

    void GraphicsSubmitInternal(
        VkCommandBuffer* buffers, uint32_t nBuffers,
        VulkanSemaphore* sem_wait, VulkanSemaphore* sem_signal,
        EStageWait stage_wait,
        VulkanFence* fence
    );
};

}

#endif