#ifndef _RENDER_RENDERER_HPP_
#define _RENDER_RENDERER_HPP_

#include "renderbackend.hpp"
#include "managers/mesh_manager.hpp"
#include "managers/texture_manager.hpp"
#include "managers/shader_list.hpp"
#include "stages/render_stage_geometry.hpp"
#include "stages/render_stage_lighting.hpp"

namespace eng
{

extern std::unique_ptr<RTextureManager> g_texture_manager;
extern std::unique_ptr<RMeshManager> g_mesh_manager;
extern std::unique_ptr<RShaderList> g_shader_list;

class RScene;

class RRenderer
{
public:
    RRenderer(std::unique_ptr<IRenderBackend>&&);
    ~RRenderer();

    void BeginFrame();
    void Render(RScene&);
    void Present();

    IRenderBackend *GetBackend() const { return backend_.get(); }
private:
    std::unique_ptr<IRenderBackend> backend_;
    std::unique_ptr<RSampler> main_sampler_;

    std::unique_ptr<RStageGeometry> geometry_stage_;
    std::unique_ptr<RStageLighting> lighting_stage_;

    std::unique_ptr<RFramebuffer> output_framebuffer_;
};

}

#endif