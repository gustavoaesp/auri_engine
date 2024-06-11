#ifndef _RENDER_SCENE_MESH_HPP_
#define _RENDER_SCENE_MESH_HPP_

#include <memory>
#include <math/quaternion.hpp>
#include <math/vector.hpp>

#include "mesh.hpp"

namespace eng
{

struct RSceneMesh
{
    RSceneMesh(
        const char *mesh_name,
        const vec3f &position,
        const Quaternion &rotation,
        const vec3f &scale
    );

    std::shared_ptr<RMesh> mesh;
    vec3f position;
    Quaternion rotation;
    vec3f scale;

    std::unique_ptr<RBuffer> uniform_buffer_transform;
};

}

#endif