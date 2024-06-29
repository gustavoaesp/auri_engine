#include "renderer.hpp"

#include "scene/scene.hpp"

#include "transform/matrix.hpp"
#include "managers/shader_list.hpp"
#include "core/global_context.hpp"

namespace eng
{

RRenderer::RRenderer(std::unique_ptr<IRenderBackend>&& backend):
    backend_(std::move(backend))
{
    g_context->texture_manager = std::make_unique<RTextureManager>(backend_.get());
    g_context->mesh_manager = std::make_unique<RMeshManager>(backend_.get());
    g_context->shader_list = std::make_unique<RShaderList>(backend_.get());

    geometry_stage_ = std::make_unique<RStageGeometry>(backend_.get(), 1600, 900);
    lighting_stage_ = std::make_unique<RStageLighting>(
        backend_.get(),
        geometry_stage_->GetGBuffer()
    );
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

void RRenderer::Render(RScene& scene)
{
    geometry_stage_->Render(scene);
    lighting_stage_->Render(scene);

    backend_->SubmitBuffers(
        std::array<RCommandBuffer*, 2>{
            geometry_stage_->GetCommandBuffer(),
            lighting_stage_->GetCommandBuffer()
        }.data(),
        2
    );
}

void RRenderer::Present()
{
    backend_->Present(lighting_stage_->GetFrame());
}

}