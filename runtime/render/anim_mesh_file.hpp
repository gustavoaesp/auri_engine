#ifndef _RENDER_ANIM_MESH_FILE_HPP_
#define _RENDER_ANIM_MESH_FILE_HPP_

#include "mesh_file.hpp"

#include <stdint.h>

#include <math/quaternion.hpp>
#include <math/vector.hpp>

namespace eng
{

static constexpr int kMaxName = 256;

static constexpr int kMaxVertices = 0x40000; // 4 * 1024 ^ 3
static constexpr int kMaxBones = 256;
static constexpr int kMaxAnimations = 64;

enum RSkinnedAnimationFileFlags {
    kUseSkeleton = 0x01,
    kUseAnimations = 0x02,
    kUseMesh = 0x04
};

struct RAnimationMeshFileHeader
{
    char singature[4];
    int bone_count;
    int submesh_count;
    int animation_count;
    int compression_enabled;
    mtx4f root_transform;
};

struct RAnimationMeshFileBone
{
    char name[kMaxName];
    int32_t parent_index;
    eng::mtx4f transform;
    eng::mtx4f offset;
};

struct RAnimationMeshFileSubmesh
{
    eng::RFileSubmeshMaterial material;
    char name[kMaxName];
    uint32_t num_vertices;
    uint32_t num_indices;
};

struct RAnimationMeshFileVertexBone
{
    char name[kMaxName];
    float weight;
};

struct RAnimationMeshFileVertex
{
    vec3f position;
    vec3f normal;
    vec2f texels;
    RAnimationMeshFileVertexBone bone_weights[4];
};

struct RAnimationMeshFileAnimation
{
    char name[kMaxName];
    int num_channels;
    int ticks_per_second;
    int total_ticks;
};

struct RAnimationMeshFileChannel
{
    char bone_name[kMaxName];
    uint32_t num_rotations;
    uint32_t num_translations;
    uint32_t num_scales;
};

struct RAnimationMeshFileChannelRotation
{
    int time_tick;
    Quaternion quat;
};

struct RAnimationMeshFileChannelVector
{
    int time_tick;
    vec3f vector;
};

void ReadAnimationMeshFile(const char *filename);

void WriteAnimationMeshFile(
    const char *filename,
    const struct RSkinnedMesh *mesh,
    const struct RSkinnedAnimation **animations,
    int num_animations, int flags);

}

#endif