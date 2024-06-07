#ifndef _RENDER_BACKEND_HPP_
#define _RENDER_BACKEND_HPP_

#include "format.hpp"
#include "vertex.hpp"
#include "primitives/buffer.hpp"
#include "primitives/cmd_pool.hpp"
#include "primitives/cmd_buffer.hpp"
#include "primitives/descriptors.hpp"
#include "primitives/framebuffer.hpp"
#include "primitives/pipeline.hpp"
#include "primitives/render_pass.hpp"
#include "primitives/sampler.hpp"
#include "primitives/shader.hpp"

#include "texture_file.hpp"

#include <stdint.h>

namespace eng
{

class IRenderBackend
{
public:
    virtual ~IRenderBackend() {}

    /*
     *  This will make an initialization into ImGui
     */
    virtual void InitializeGUI() = 0;

    virtual void BeginFrame() = 0;
    virtual void BeginRender() = 0;
    /*
     *  It will use the final resulting image (From the fb object) as a texture
     *  and just render it to the screen (this will be implementation specific)
     */
    virtual void Present(RFramebuffer* final_image) = 0;

    virtual void Finalize() = 0;

    virtual RBuffer *CreateBuffer(
        RBufferUsage usage,
        size_t size,
        void *contents
    ) = 0;

    virtual void UpdateBuffer(
        RBuffer *buffer,
        void *data,
        size_t start_offset,
        size_t size
    ) = 0;

    virtual RTexture *CreateImage2D(
        uint32_t width, uint32_t height,
        EFormat pixel_fmt
    ) = 0;

    virtual RTexture *CreateTexture2D(
        const RTextureFile *file
    ) = 0;

    /*
     *
     */
    virtual RRenderPass *CreateRenderPass(
        const RRenderPassAttachment* color_attachments,
        int num_color_attachments,
        const RRenderPassAttachment* depth // null -> no depth attachment
    ) = 0;

    /*
     *  The RFramebuffer assumes ownership of these pointers with unique_ptr
     *  depth_stencil can be null
     */
    virtual RFramebuffer *CreateFramebuffer(
        RRenderPass*,
        RTexture** images,
        int num_images,
        RTexture* depth_stencil
    ) = 0;

    virtual RPipeline *CreatePipeline(
        const RRenderPass *,
        const RBlendState *,
        const RDepthStencilState *depth_state,
        const RShader **shaders,
        int num_shaders,
        RVertexType vertex_type,
        RVertexType instance_type,
        RDescriptorLayout **,
        int num_descriptor_set_layouts
    ) = 0;

    virtual RShader *CreateShader(
        const char* filename,
        RShaderPipelineBind
    ) = 0;

    /*
     *  The command pool will allocate any new command buffer
     */
    virtual RCommandPool *CreateCommandPool() = 0;

    /*
     *  SHOULD BE called only once per frame
     */
    virtual void SubmitBuffers(
        RCommandBuffer**,
        uint32_t num_buffers
    ) = 0;

    /*
     *  Descriptor set stuff
     */
    virtual RDescriptorLayout *CreateDescriptorLayout(
        const RDescriptorLayoutBinding*,
        uint32_t num_bindings
    ) = 0;

    virtual RDescriptorPool *CreateDescriptorPool(
        uint32_t max_sets,
        RDescriptorLayoutBindingType
    ) = 0;

    virtual RSampler *CreateSampler(
        RSamplerFilterMode,
        RSamplerAddressMode
    ) = 0;

protected:
    IRenderBackend() {}
};

}

#endif