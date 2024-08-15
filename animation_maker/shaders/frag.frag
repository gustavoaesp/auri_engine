#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler0;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;

void main()
{
    //outColor = vec4(fragColor, 1.0f);
    outColor = texture(texSampler0, fragColor.xy);
}