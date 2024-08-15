#version 450

layout(set = 1, binding = 0) uniform sampler2D g_diffuse;
layout(set = 1, binding = 1) uniform sampler2D g_normal;

layout(set = 0, binding = 0) uniform uAmbientLight
{
    vec4 ambient_color;
};

layout (location = 0) in vec2 tex_uv;
layout (location = 0) out vec4 outColor;

void main() {
    vec3 diffuse = texture(g_diffuse, tex_uv).xyz;
    outColor = vec4(diffuse * ambient_color.xyz, 1.0f);
}