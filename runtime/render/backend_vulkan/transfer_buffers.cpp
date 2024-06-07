#include "backend_vulkan/transfer_buffers.hpp"

#include <string.h>

namespace eng
{

VulkanTransferBuffers::VulkanTransferBuffers(
    VulkanDevice *vulkan_device, VmaAllocator alloc, std::unique_ptr<VulkanCommandPool>&& cmd_pool
): vulkan_device_ref_(vulkan_device), cmd_pool_(std::move(cmd_pool)), vk_allocator_ref(alloc)
{
    cmd_buffer_ = std::unique_ptr<VulkanCommandBuffer>(
        static_cast<VulkanCommandBuffer*>(cmd_pool_->CreateCommandBuffer(true))
    );
}

VulkanTransferBuffers::~VulkanTransferBuffers()
{
}

VulkanTexture *VulkanTransferBuffers::CreateTexture2D(const RTextureFile *file, bool mips_enabled)
{
    std::lock_guard<std::mutex> lock(transfer_mutex_);

    VkBuffer vk_buffer;
    VmaAllocation buffer_allocation;

    VkBufferCreateInfo buffer_info{};
    VmaAllocationCreateInfo alloc_info{};

    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = file->data.size();
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateBuffer(
        vk_allocator_ref,
        &buffer_info,
        &alloc_info,
        &vk_buffer,
        &buffer_allocation,
        nullptr
    );

    void *buffer_data;
    vmaMapMemory(vk_allocator_ref, buffer_allocation, &buffer_data);
    memcpy(buffer_data, file->data.data(), file->data.size());
    vmaUnmapMemory(vk_allocator_ref, buffer_allocation);

    VulkanTexture *tex = new VulkanTexture(
        vulkan_device_ref_->get(),
        vk_allocator_ref,
        EFormat::kFormat_R8G8B8A8_UNORM,
        file->header.width, file->header.height
    );
    uint32_t mips_count = (uint32_t)std::floor(
        std::log2(
            std::min(file->header.width, file->header.height)
        )
    ) + 1;

    VmaAllocationCreateInfo img_alloc_info{};
    img_alloc_info.flags = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = file->header.width;
    imageInfo.extent.height = file->header.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mips_count;
    imageInfo.arrayLayers = 1;

    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    vmaCreateImage(
        vk_allocator_ref,
        &imageInfo,
        &img_alloc_info,
        &tex->vk_image,
        &tex->vk_allocation,
        nullptr
    );

    cmd_buffer_->Reset();
    cmd_buffer_->BeginRecord();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = tex->vk_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;                    
    barrier.subresourceRange.levelCount = mips_count;    
    barrier.subresourceRange.baseArrayLayer = 0;    
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier
    (
        cmd_buffer_->vk_command_buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {
        (uint32_t)file->header.width,
        (uint32_t)file->header.height,
        1
    };
    vkCmdCopyBufferToImage
    (
        cmd_buffer_->vk_command_buffer,
        vk_buffer,
        tex->vk_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region
    );

    /*
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier
    (
        cmd_buffer.get(),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    */

    // Reuse barrier for blit cmds for mipmap creation
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = file->header.width;
    int32_t mipHeight = file->header.height;

    for (int i = 1; i < mips_count; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier
        (
            cmd_buffer_->vk_command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth / 2, mipHeight / 2, 1 };
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        vkCmdBlitImage
        (
            cmd_buffer_->vk_command_buffer,
            tex->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            tex->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR
        );

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier
        (
            cmd_buffer_->vk_command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        mipWidth /= 2;
        mipHeight /= 2;
        if (!mipWidth || !mipHeight ) {
            break;
        }
    }

    barrier.subresourceRange.baseMipLevel = mips_count - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier
    (
        cmd_buffer_->vk_command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    cmd_buffer_->EndRecord();

    vulkan_device_ref_->GraphicsSubmit(
        cmd_buffer_.get(),
        nullptr, nullptr,
        EStageWait::kStageColorAttachment,
        nullptr
    );

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = tex->vk_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mips_count;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vulkan_device_ref_->get(), &viewInfo, nullptr, &tex->vk_image_view) != VK_SUCCESS) {
        //
    }

    vulkan_device_ref_->WaitIdle();

    vmaDestroyBuffer(vk_allocator_ref, vk_buffer, buffer_allocation);

    return tex;
}

}