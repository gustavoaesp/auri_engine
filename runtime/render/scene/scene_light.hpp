#ifndef _RENDER_SCENE_LIGHT_HPP_
#define _RENDER_SCENE_LIGHT_HPP_

#include <math/vector.hpp>
#include <memory>

#include "render/primitives/framebuffer.hpp"

namespace eng
{

enum class RSceneLightType
{
    kLightDirectional
};

struct RSceneLight
{
    vec3f direction;
    vec3f position;
    float angle;

    // common for all lights
    vec3f color;
    float intensity; // 1.0f default
    float range;

    RSceneLightType type;

    std::unique_ptr<RBuffer> uniform_buffer;
};

}

#endif