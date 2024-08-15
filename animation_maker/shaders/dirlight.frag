#version 450

layout(set = 1, binding= 0) uniform sampler2D g_diffuse;
layout(set = 1, binding= 1) uniform sampler2D g_normal;

layout(set = 0, binding = 0) uniform uDirectionalLight
{
    vec4 direction;
    vec4 color;
};

layout (location = 0) in vec2 tex_uv;
layout (location = 0) out vec4 outColor;

void main() {
    vec3 diffuse = texture(g_diffuse, tex_uv).xyz;
    vec3 normal = texture(g_normal, tex_uv).xyz;
    normal = normalize(normal * 2 - 1.0f);

    vec3 unit_direction = normalize(direction.xyz);

    outColor = vec4(dot(normal, -direction.xyz) * color.xyz * diffuse, 1.0f);
}