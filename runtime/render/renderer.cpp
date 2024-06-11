#include "renderer.hpp"

#include "scene/scene.hpp"

#include "transform/matrix.hpp"

namespace eng
{

std::unique_ptr<RTextureManager> g_texture_manager;
std::unique_ptr<RMeshManager> g_mesh_manager;

RRenderer::RRenderer(std::unique_ptr<IRenderBackend>&& backend):
    backend_(std::move(backend))
{
    g_texture_manager = std::make_unique<RTextureManager>(backend_.get());
    g_mesh_manager = std::make_unique<RMeshManager>(backend_.get());

    geometry_stage_ = std::make_unique<RStageGeometry>(backend_.get(), 1600, 900);
}

RRenderer::~RRenderer()
{
    backend_->Finalize();
}

void RRenderer::BeginFrame()
{
    backend_->BeginFrame();
    backend_->BeginRender();
}

void RRenderer::Render(const RScene& scene)
{
    geometry_stage_->Render(scene);

    backend_->SubmitBuffers(
        std::array<RCommandBuffer*, 1>{
            geometry_stage_->GetCommandBuffer()
        }.data(),
        1
    );
}

void RRenderer::Present()
{
    backend_->Present(geometry_stage_->GetGBuffer());
}



}