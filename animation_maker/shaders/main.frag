#version 450

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;

void main()
{
    outColor = texture(diffuseSampler, inUV);
    outNormal = vec4((normalize(inNormal) + 1.0f) / 2, 1.0f);
}