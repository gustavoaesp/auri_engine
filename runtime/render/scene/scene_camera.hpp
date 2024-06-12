#ifndef _RENDER_SCENE_CAMERA_HPP_
#define _RENDER_SCENE_CAMERA_HPP_

#include "primitives/buffer.hpp"

#include <math/vector.hpp>

#include <memory>

namespace eng
{

struct RSceneCamera
{
    RSceneCamera(const vec3f& pos, const vec3f& look_pos, const vec3f& up_vector, float fovy);
    vec3f position;
    vec3f look_pos;
    vec3f up;

    float fovy;
};

}

#endif