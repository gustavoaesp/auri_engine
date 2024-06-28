#ifndef _RENDER_STAGE_LIGHTING_HPP_
#define _RENDER_STAGE_LIGHTING_HPP_

#include "renderbackend.hpp"
#include "scene/scene_light.hpp"
#include "stages/render_stage.hpp"

namespace eng
{

struct RLightDirectionalUniform
{
    vec3f direction;
    float __padding;
    vec3f color;
    float intensity;
};

class RStageLighting : public IRenderStage
{
public:
    RStageLighting(
        IRenderBackend *,
        RFramebuffer *g_buffer);
    ~RStageLighting();

    void Render(RScene&) override;

    RFramebuffer *GetFrame() const { return output_frame_.get(); }

private:
    IRenderBackend *backend_ref_;
    RFramebuffer *gbuffer_ref_;

    std::unique_ptr<RShader> directional_vertex_shader_;
    std::unique_ptr<RShader> directional_pixel_shader_;
    std::unique_ptr<RPipeline> directional_pipeline_;

    std::unique_ptr<RFramebuffer> output_frame_;

    std::unique_ptr<RSampler> main_sampler_;
    std::unique_ptr<RBuffer> quad_vertex_buffer_;
    std::unique_ptr<RBuffer> quad_index_buffer_;

    void ProcessDirectionalLight(
        RSceneLight *light,
        RDescriptorSet *textures, RDescriptorSet *buffers
    );
};

}

#endif