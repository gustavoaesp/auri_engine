#include "managers/texture_manager.hpp"

namespace eng
{

RTextureManager::RTextureManager(IRenderBackend *backend_ref):
    backend_ref_(backend_ref)
{
}

RTexture *RTextureManager::Load(const char *filename)
{
    RTextureFile texture_file = RTextureRead(filename);

    return backend_ref_->CreateTexture2D(&texture_file);
}

}