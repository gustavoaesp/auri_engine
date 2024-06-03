#ifndef _RENDER_BACKEND_VULKAN_CMD_BUFFER_HPP_
#define _RENDER_BACKEND_VULKAN_CMD_BUFFER_HPP_

#include <vulkan/vulkan.h>

#include "primitives/cmd_buffer.hpp"

namespace eng
{

struct VulkanCommandBuffer : public RCommandBuffer
{
    VulkanCommandBuffer(VkCommandBuffer);
    ~VulkanCommandBuffer() override;

    virtual void Reset() override;
    virtual void BeginRecord() override;
    virtual void BeginRecord(
        const RRenderPass*,
        const RFramebuffer*
    ) override;
    virtual void EndRecord() override;

    virtual void CmdBeginRenderPass(
        const RRenderPass*, const RFramebuffer*,
        const vec4f* clear_colors, int num_clear_colors,
        uint8_t stencil,
        bool from_secondary_buffers
    ) override;

    virtual void CmdEndRenderPass() override;

    virtual void CmdBindPipeline(const RPipeline*) override;

    virtual void CmdBindVertexBuffer(
        const RBuffer*,
        uint32_t binding_index,
        uint32_t offset_bytes
    ) override;

    virtual void CmdBindIndexBuffer(
        const RBuffer*,
        uint32_t offset_bytes
    ) override;

    virtual void CmdBindDescriptorSets(
        const RPipeline*,
        const RDescriptorSet**,
        uint32_t count
    ) override;

    virtual void CmdDraw(
        uint32_t vertex_count,
        uint32_t first_vertex
    ) override;

    virtual void CmdDrawIndexed(
        uint32_t index_count,
        uint32_t first_index,
        uint32_t first_vertex
    ) override;

    virtual void CmdSetScissor(
        int32_t offset_x, int32_t offset_y,
        int32_t width, int32_t height
    ) override;

    virtual void CmdSetViewport(
        float x, float y,
        int32_t width, int32_t height
    ) override;

    virtual void CmdExecuteBuffers(
        RCommandBuffer**,
        uint32_t num_buffers
    ) override;

    VkCommandBuffer vk_command_buffer;
};

}

#endif