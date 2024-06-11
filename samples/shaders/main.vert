#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (set = 0, binding = 0) uniform uViewProjection
{
    mat4 view_proj_mtx;
};
layout (set = 0, binding = 1) uniform uModel
{
    mat4 model_mtx;
};

layout (location = 0) out vec3 outNorm;
layout (location = 1) out vec2 outUV;

void main() {
    outNorm = outNorm;
    outUV = inUV;
    gl_Position = view_proj_mtx * model_mtx * vec4(inPosition, 1.0f);
    //gl_Position = view_proj_mtx * vec4(inPosition, 1.0f);
    //gl_Position = vec4(inPosition, 1.0f);
}