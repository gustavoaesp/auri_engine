#include "mesh_manager.hpp"

namespace eng
{

RMeshManager::RMeshManager(IRenderBackend *backend_ref):
    backend_ref_(backend_ref)
{
}

RMesh *RMeshManager::Load(const char *filename)
{
    return nullptr;
}

}