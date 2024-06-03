#ifndef _RENDER_CMD_BUFFER_HPP_
#define _RENDER_CMD_BUFFER_HPP_

#include <stdint.h>
#include <math/vector.hpp>

namespace eng
{

class RBuffer;
class RFramebuffer;
class RPipeline;
class RRenderPass;
class RDescriptorSet;

class RCommandBuffer
{
public:
    virtual ~RCommandBuffer() {}

    virtual void Reset() = 0;
    virtual void BeginRecord() = 0;
    virtual void BeginRecord(
        const RRenderPass*,
        const RFramebuffer*
    ) = 0;
    virtual void EndRecord() = 0;

    virtual void CmdBeginRenderPass(
        const RRenderPass*, const RFramebuffer*,
        const vec4f* clear_colors, int num_clear_colors,
        uint8_t stencil, bool from_secondary_buffers
    ) = 0;

    virtual void CmdEndRenderPass() = 0;

    virtual void CmdBindPipeline(const RPipeline*) = 0;

    virtual void CmdBindVertexBuffer(
        const RBuffer*,
        uint32_t binding_index,
        uint32_t offset_bytes
    ) = 0;

    virtual void CmdBindIndexBuffer(
        const RBuffer*,
        uint32_t offset_bytes
    ) = 0;

    virtual void CmdBindDescriptorSets(
        const RPipeline*,
        const RDescriptorSet**,
        uint32_t count
    ) = 0;

    virtual void CmdDraw(
        uint32_t vertex_count,
        uint32_t first_vertex
    ) = 0;

    virtual void CmdDrawIndexed(
        uint32_t index_count,
        uint32_t first_index,
        uint32_t first_vertex
    ) = 0;

    virtual void CmdSetScissor(
        int32_t offset_x, int32_t offset_y,
        int32_t width, int32_t height
    ) = 0;

    virtual void CmdSetViewport(
        float x, float y,
        int32_t width, int32_t height
    ) = 0;

    virtual void CmdExecuteBuffers(
        RCommandBuffer**,
        uint32_t num_buffers
    ) = 0;

protected:
    RCommandBuffer() {}
};

}

#endif