#include "mesh_manager.hpp"
#include "mesh_file.hpp"

namespace eng
{

RMeshManager::RMeshManager(IRenderBackend *backend_ref)
{
}

RMesh *RMeshManager::Load(const char *filename)
{
    return RFileMeshRead(filename);
}

}