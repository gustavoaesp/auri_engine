#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in uvec4 bones;
layout (location = 4) in vec4 weights;

layout (set = 0, binding = 0) uniform uViewProjection
{
    mat4 view_proj_mtx;
};
layout (set = 0, binding = 1) uniform uModel
{
    mat4 model_mtx;
};
layout (set = 0, binding = 2) uniform uBoneMatrices 
{
    mat4 bone_matrices[128];
};

layout (location = 0) out vec3 outNorm;
layout (location = 1) out vec2 outUV;

void main() {
    mat4 weighted_mat =
        weights.x * bone_matrices[bones.x] +
        weights.y * bone_matrices[bones.y] +
        weights.z * bone_matrices[bones.z] +
        weights.w * bone_matrices[bones.w];

    /*vec4 bone_position = weights.x * bone_matrices[bones.x] * vec4(inPosition, 1.0f);
    bone_position += weights.y * bone_matrices[bones.y] * vec4(inPosition, 1.0f);
    bone_position += weights.z * bone_matrices[bones.z] * vec4(inPosition, 1.0f);
    bone_position += weights.w * bone_matrices[bones.w] * vec4(inPosition, 1.0f);*/
    vec4 bone_position = weighted_mat * vec4(inPosition, 1.0f);

    //vec4 bone_position = vec4(inPosition, 1.0f);

    mat4 model_inv_trans = inverse(transpose(model_mtx * weighted_mat));
    outNorm = (model_inv_trans * vec4(inNormal, 0.0f)).xyz;
    outUV = inUV;
    gl_Position = view_proj_mtx * model_mtx * bone_position;
}