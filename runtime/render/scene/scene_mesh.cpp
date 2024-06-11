#include "scene/scene_mesh.hpp"
#include "renderer.hpp"

namespace eng
{

RSceneMesh::RSceneMesh(const char *mesh_name,
    const vec3f &_pos, const Quaternion &_rot,
    const vec3f &_scale
): position(_pos), rotation(_rot), scale(_scale)
{
    mesh = g_mesh_manager->Get(mesh_name);
}

}