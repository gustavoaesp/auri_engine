#pragma once

#include <vulkan/vulkan.h>
//#include <vulkan/pipeline.hpp>
#include <math/vector.hpp>

namespace eng
{

enum class RVertexType
{
    kVertexPos3Col4 = 0,
    kVertexPos3Nor3Tex2 = 1,
    kNoFormat = 0x7fffffff
};

struct Vertex
{
    vec3f position;
    vec4f color;
};

struct Vertex_NorTuv
{
    vec3f position;
    vec3f normal;
    vec2f texel;
};

}
