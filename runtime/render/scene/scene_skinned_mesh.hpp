#ifndef _RENDER_SCENE_SKINNED_MESH_HPP_
#define _RENDER_SCENE_SKINNED_MESH_HPP_

#include <math/quaternion.hpp>
#include "skinned_mesh.hpp"
#include "skinned_animation.hpp"

namespace eng
{

struct RSceneSkinnedMesh
{
    RSceneSkinnedMesh(
        std::shared_ptr<RSkinnedMesh> &,
        std::shared_ptr<RSkinnedAnimation> &,
        const vec3f &position,
        const Quaternion &rotation,
        const vec3f &scale
    );

    std::shared_ptr<RSkinnedMesh> mesh;
    std::shared_ptr<RSkinnedAnimation> animation;

    vec3f position;
    Quaternion rotation;
    vec3f scale;

    std::vector<mtx4f> bones;

    std::unique_ptr<RBuffer> uniform_buffer_transform;
    std::unique_ptr<RBuffer> uniform_bone_matrices;

    void Tick(float dt);

    float current_time;
};

}

#endif