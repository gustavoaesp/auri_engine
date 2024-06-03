#version 450

layout(set = 0, binding= 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 tex_uv;
layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, tex_uv);
}