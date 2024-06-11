#ifndef _RENDER_RENDERER_HPP_
#define _RENDER_RENDERER_HPP_

#include "renderbackend.hpp"
#include "managers/mesh_manager.hpp"
#include "managers/texture_manager.hpp"

namespace eng
{

extern std::unique_ptr<RTextureManager> g_texture_manager;
extern std::unique_ptr<RMeshManager> g_mesh_manager;

class RScene;

class RRenderer
{
public:
    RRenderer(std::unique_ptr<IRenderBackend>&&);
    ~RRenderer();

    void BeginFrame();
    void Render(const RScene&);
    void Present();

    IRenderBackend *GetBackend() const { return backend_.get(); }
private:
    std::unique_ptr<IRenderBackend> backend_;

    std::unique_ptr<RRenderPass> render_pass_geometry_;
    std::unique_ptr<RShader> main_vert_;
    std::unique_ptr<RShader> main_frag_;
    std::unique_ptr<RPipeline> pipeline_geometry_;
    std::unique_ptr<RFramebuffer> main_framebuffer_;

    std::unique_ptr<RDescriptorLayout> desc_layout_buffers_;
    std::unique_ptr<RDescriptorLayout> desc_layout_textures_;

    std::unique_ptr<RDescriptorPool> descriptor_pool_buffers_;
    std::unique_ptr<RDescriptorPool> descriptor_pool_textures_;

    std::vector<std::unique_ptr<RDescriptorSet>> descriptor_sets_buffer_;
    std::vector<std::unique_ptr<RDescriptorSet>> descriptor_sets_texture_;

    std::unique_ptr<RCommandPool> cmd_main_pool_;
    std::unique_ptr<RCommandBuffer> cmd_main_buffer_;

    std::unique_ptr<RBuffer> view_projection_uniform_;

    std::unique_ptr<RSampler> main_sampler_;
};

}

#endif