#version 450

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main()
{
    outColor = texture(diffuseSampler, inUV);
}