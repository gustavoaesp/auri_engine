#ifndef _RENDER_RENDER_STAGE_GEOMETRY_HPP_
#define _RENDER_RENDER_STAGE_GEOMETRY_HPP_

#include "renderbackend.hpp"
#include "stages/render_stage.hpp"

namespace eng
{

struct RSubmesh;

class RStageGeometry : public IRenderStage
{
public:
    RStageGeometry(IRenderBackend *, uint32_t width, uint32_t height);
    ~RStageGeometry() override;

    void Render(const RScene&) override;

    RFramebuffer *GetGBuffer() const { return g_buffer_.get(); }

private:
    void RenderSubmesh(RSubmesh *);
    std::unique_ptr<RFramebuffer> g_buffer_;

    IRenderBackend *backend_ref_;
    std::unique_ptr<RShader> main_vert_;
    std::unique_ptr<RShader> main_frag_;
    std::unique_ptr<RPipeline> main_pipeline_;

    std::unique_ptr<RDescriptorLayout> desc_layout_buffers_;
    std::unique_ptr<RDescriptorLayout> desc_layout_textures_;

    std::unique_ptr<RDescriptorPool> desc_pool_buffers_;
    std::unique_ptr<RDescriptorPool> desc_pool_textures_;

    std::vector<std::unique_ptr<RDescriptorSet>> descriptor_sets_buffer_;
    std::vector<std::unique_ptr<RDescriptorSet>> descriptor_sets_texture_;

    std::unique_ptr<RSampler> main_sampler_;

    std::unique_ptr<RBuffer> view_projection_uniform_;
};

}

#endif