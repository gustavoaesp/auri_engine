#include "backend_vulkan/primitives/vk_buffer.hpp"
#include "backend_vulkan/primitives/vk_cmd_buffer.hpp"
#include "backend_vulkan/primitives/vk_pipeline.hpp"
#include "backend_vulkan/primitives/vk_render_pass.hpp"
#include "backend_vulkan/primitives/vk_framebuffer.hpp"
#include "vk_cmd_buffer.hpp"

#include <iostream>

namespace eng
{

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandBuffer vk_buffer):
    vk_command_buffer(vk_buffer)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
}

void VulkanCommandBuffer::Reset()
{
    vkResetCommandBuffer(vk_command_buffer, 0);
}

void VulkanCommandBuffer::BeginRecord()
{
    VkCommandBufferBeginInfo vk_begin_info{};
    vk_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vk_begin_info.flags = 0;
    vk_begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(vk_command_buffer, &vk_begin_info) != VK_SUCCESS) {
        //ExitError("Can't begin recording command buffer");
        // TODO error
        std::cerr << "WTF potter!!\n";
    }
}

void VulkanCommandBuffer::BeginRecord(const RRenderPass *, const RFramebuffer *)
{
}

void VulkanCommandBuffer::EndRecord()
{
    if (vkEndCommandBuffer(vk_command_buffer) != VK_SUCCESS) {
        //ExitError("Can't end recording command buffer");
        // TODO error
    }
}

void VulkanCommandBuffer::CmdBeginRenderPass(
    const RRenderPass *render_pass,
    const RFramebuffer *framebuffer,
    const vec4f *clear_colors, int num_clear_colors,
    uint8_t stencil,
    bool from_secondary_buffers)
{
    const VulkanRenderPass *vk_render_pass = static_cast<const VulkanRenderPass*>(render_pass);
    const VulkanFramebuffer *vk_framebuffer = static_cast<const VulkanFramebuffer*>(framebuffer);

    VkRenderPassBeginInfo rpBeginInfo{};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = vk_render_pass->vk_render_pass;
    rpBeginInfo.framebuffer = vk_framebuffer->vk_framebuffer;
    rpBeginInfo.renderArea.offset = {0, 0};
    rpBeginInfo.renderArea.extent.width = framebuffer->GetWidth();
    rpBeginInfo.renderArea.extent.height = framebuffer->GetHeight();
    rpBeginInfo.clearValueCount = num_clear_colors;
    rpBeginInfo.pClearValues = (const VkClearValue*)clear_colors;
    vkCmdBeginRenderPass(
        vk_command_buffer,
        &rpBeginInfo,
        (from_secondary_buffers) ? 
            VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE
    );
}

void VulkanCommandBuffer::CmdEndRenderPass()
{
    vkCmdEndRenderPass(vk_command_buffer);
}

void VulkanCommandBuffer::CmdBindPipeline(const RPipeline *pipeline)
{
    const VulkanPipeline *vk_pipeline = static_cast<const VulkanPipeline*>(pipeline);
    vkCmdBindPipeline(
        vk_command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        vk_pipeline->vk_pipeline
    );
}

void VulkanCommandBuffer::CmdBindVertexBuffer(
    const RBuffer *vertex_buffer,
    uint32_t binding_index,
    uint32_t offset_bytes)
{
    const VulkanBuffer *vk_buffer = static_cast<const VulkanBuffer*>(vertex_buffer);
    VkBuffer vertex_buffer_array[] = { vk_buffer->vk_buffer };
    VkDeviceSize offset_array[] = { offset_bytes };

    vkCmdBindVertexBuffers(
        vk_command_buffer,
        binding_index,
        1,
        vertex_buffer_array,
        offset_array
    );
}

void VulkanCommandBuffer::CmdBindIndexBuffer(const RBuffer *index_buffer, uint32_t offset_bytes)
{
    const VulkanBuffer *vk_buffer = static_cast<const VulkanBuffer*>(index_buffer);

    vkCmdBindIndexBuffer(
        vk_command_buffer,
        vk_buffer->vk_buffer,
        offset_bytes,
        VK_INDEX_TYPE_UINT32
    );
}

void VulkanCommandBuffer::CmdBindDescriptorSets(
    const RPipeline* pipeline,
    const RDescriptorSet **sets,
    uint32_t count)
{
    std::array<VkDescriptorSet, 8> vk_sets{};
    const VulkanPipeline *vk_pipeline = static_cast<const VulkanPipeline*>(pipeline);

    for (int i = 0; i < count; i++) {
        const VulkanDescriptorSet *vk_set = static_cast<const VulkanDescriptorSet*>(sets[i]);
        vk_sets[i] = vk_set->vk_descriptor_set;
    }

    vkCmdBindDescriptorSets(
        vk_command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        vk_pipeline->vk_pipeline_layout,
        0, count,
        vk_sets.data(),
        0, nullptr
    );
}

void VulkanCommandBuffer::CmdDraw(uint32_t vertex_count, uint32_t first_vertex)
{
    vkCmdDraw(vk_command_buffer, vertex_count, 1, first_vertex, 0);
}

void VulkanCommandBuffer::CmdDrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
{
    vkCmdDrawIndexed(
        vk_command_buffer,
        index_count,
        1,
        first_index,
        first_vertex,
        0
    );
}

void VulkanCommandBuffer::CmdSetScissor(int32_t offset_x, int32_t offset_y, int32_t width, int32_t height)
{
    VkRect2D scissor{};
    scissor.offset = {offset_x, offset_y};
    scissor.extent.width = width;
    scissor.extent.height = height;

    vkCmdSetScissor(vk_command_buffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::CmdSetViewport(float x, float y, int32_t width, int32_t height)
{
    VkViewport vk_viewport{};
    vk_viewport.x = x;
    vk_viewport.y = y;
    vk_viewport.width = static_cast<float>(width);
    vk_viewport.height = static_cast<float>(height);
    vk_viewport.minDepth = 0.0f;
    vk_viewport.maxDepth = 1.0f;
    vkCmdSetViewport(
        vk_command_buffer,
        0, 1,
        &vk_viewport
    );
}

void VulkanCommandBuffer::CmdExecuteBuffers(RCommandBuffer **cmd_buffers, uint32_t num_buffers)
{
    std::array<VkCommandBuffer, 64> vk_cmd_buffers_array;
    for (int i = 0; i < num_buffers; ++i) {
        VulkanCommandBuffer *cmd_buffer = static_cast<VulkanCommandBuffer*>(cmd_buffers[i]);
        vk_cmd_buffers_array[i]= cmd_buffer->vk_command_buffer;
    }

    vkCmdExecuteCommands(
        vk_command_buffer,
        num_buffers,
        vk_cmd_buffers_array.data()
    );
}

}