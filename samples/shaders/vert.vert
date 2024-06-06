#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform uMVP
{
    mat4 mvp;
};

void main() {
    gl_Position = mvp * vec4(inPosition, 1.0f);
    fragColor = inColor.xyz;
}