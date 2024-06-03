#ifndef _RENDER_BACKEND_VULKAN_CMD_POOL_HPP_
#define _RENDER_BACKEND_VULKAN_CMD_POOL_HPP_

#include <vulkan/vulkan.h>

#include "primitives/cmd_pool.hpp"

namespace eng
{

class VulkanCommandPool : public RCommandPool
{
public:
    VulkanCommandPool(VkDevice);
    ~VulkanCommandPool() override;

    RCommandBuffer *CreateCommandBuffer(bool primary) override;

private:
    VkDevice vk_device_ref_;
    VkCommandPool vk_pool_;
};

}

#endif