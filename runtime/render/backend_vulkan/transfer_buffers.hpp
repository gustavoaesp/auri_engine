#ifndef _RENDER_BACKEND_VULKAN_TRANSFER_QUEUE_HPP_
#define _RENDER_BACKEND_VULKAN_TRANSFER_QUEUE_HPP_

#include <mutex>
#include <vector>

#include "backend_vulkan/primitives/vk_cmd_pool.hpp"
#include "backend_vulkan/primitives/vk_buffer.hpp"
#include "backend_vulkan/primitives/vk_texture.hpp"

#include "render/texture_file.hpp"

namespace eng
{

class VulkanTransferBuffers
{
public:
    VulkanTransferBuffers(VulkanDevice*, VmaAllocator alloc_ref, std::unique_ptr<VulkanCommandPool>&&);
    ~VulkanTransferBuffers();

    VulkanTexture *CreateTexture2D(
        const RTextureFile*,
        bool mips
    );

private:
    std::unique_ptr<VulkanCommandPool> cmd_pool_;
    std::unique_ptr<VulkanCommandBuffer> cmd_buffer_;

    std::mutex transfer_mutex_;

    bool transfer_contents_;

    VulkanDevice *vulkan_device_ref_;
    VmaAllocator vk_allocator_ref;
};

}

#endif