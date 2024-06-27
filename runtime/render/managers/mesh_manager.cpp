#include "mesh_manager.hpp"
#include "mesh_file.hpp"

namespace eng
{

RMeshManager::RMeshManager(IRenderBackend *backend_ref):
    backend_ref_(backend_ref)
{
}

RMesh *RMeshManager::Load(const char *filename)
{
    return RFileMeshRead(backend_ref_, filename);
}

}