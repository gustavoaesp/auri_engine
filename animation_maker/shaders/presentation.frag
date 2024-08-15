#version 450

layout(set = 0, binding= 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 tex_uv;
layout (location = 0) out vec4 outColor;

void main() {
    const float gamma = 1.2;
    vec3 hdr_color = texture(texSampler, tex_uv).rgb;
    vec3 mapped = hdr_color / (hdr_color + vec3(1.0f));
    mapped = pow(mapped, vec3(1.0f / gamma));
    outColor = vec4(mapped, 1.0f);
}