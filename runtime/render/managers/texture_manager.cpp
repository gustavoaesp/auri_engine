#include "managers/texture_manager.hpp"

#include "core/global_context.hpp"
#include "renderer.hpp"

namespace eng
{

RTextureManager::RTextureManager(IRenderBackend *backend_ref)
{
}

RTexture *RTextureManager::Load(const char *filename)
{
    RTextureFile texture_file = RTextureRead(filename);

    return g_context->renderer->GetBackend()->CreateTexture2D(&texture_file);
}

}