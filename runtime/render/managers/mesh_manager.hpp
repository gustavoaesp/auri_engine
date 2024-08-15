#ifndef _RENDER_MESH_MANAGER_HPP_
#define _RENDER_MESH_MANAGER_HPP_

#include "renderbackend.hpp"
#include "mesh.hpp"
#include "managers/manager_template.hpp"

#include <mutex>
#include <unordered_map>

namespace eng
{

class RMeshManager : public TResourceManager<RMesh>
{
public:
    RMeshManager(IRenderBackend*);

protected:
    RMesh *Load(const char*) override;
};

}

#endif