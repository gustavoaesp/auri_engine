#include "anim_mesh_file.hpp"

#include "skinned_mesh.hpp"
#include "skinned_animation.hpp"

#include <string.h>

namespace eng
{

static void BuildFileVertices(
    const RSkinnedMesh *mesh,
    const RSkinnedSubmesh& submesh,
    std::vector<RAnimationMeshFileVertex>& vertices)
{
    for (const auto &vertex: submesh.vertices) {
        vertices.push_back(
            RAnimationMeshFileVertex{
                .position = vertex.position,
                .normal = vertex.normal,
                .texels = vertex.texel
            }
        );
        for (int i = 0; i < 4; ++i) {
            strcpy(
                vertices.back().bone_weights[i].name,
                mesh->GetBone(vertex.bones[i])->name.c_str()
            );
            vertices.back().bone_weights[i].weight = vertex.weights[i];
        }
    }
}

static void WriteSubmeshes(std::ofstream &outfile, const RSkinnedMesh *mesh)
{
    for (int i = 0; i < mesh->GetSubmeshCount(); ++i) {
        const RSkinnedSubmesh *submesh = mesh->GetSubmesh(i);
        RFileSubmesh file_submesh_header;
        strcpy(
            file_submesh_header.material.diffuse_tex,
            submesh->material.diffuse_filename.c_str()
        );
        file_submesh_header.num_vertices = submesh->vertices.size();
        file_submesh_header.num_indices = submesh->indices.size();
        outfile.write((char*)&file_submesh_header, sizeof(RFileSubmesh));
    }

    for (int i = 0; i < mesh->GetSubmeshCount(); ++i) {
        const RSkinnedSubmesh *submesh = mesh->GetSubmesh(i);

        std::vector<RAnimationMeshFileVertex> file_vertices;
        file_vertices.reserve(1024);
        BuildFileVertices(mesh, *submesh, file_vertices);
        outfile.write(
            (char*)file_vertices.data(),
            sizeof(RAnimationMeshFileVertex) * file_vertices.size()
        );
    }
}

static void WriteBones(std::ofstream &outfile, const RSkinnedMesh *mesh)
{
    for (int i = 0; i < mesh->GetBoneCount(); ++i) {
        const RSkinnedMesh::Bone *bone = mesh->GetBone(i);
        RAnimationMeshFileBone output_bone{};

        strcpy(output_bone.name, bone->name.c_str());
        output_bone.offset = bone->offset_mtx;
        output_bone.parent_index = bone->parent_idx;
        output_bone.transform = bone->local_mtx;

        outfile.write((char*)&output_bone, sizeof(RAnimationMeshFileBone));
    }
}

static void WriteAnimationChannel(
    std::ofstream &outfile,
    const RAnimationChannel *channel)
{
    RAnimationMeshFileChannel output_channel{};
    strcpy(output_channel.bone_name, channel->bone->name.c_str());
    output_channel.num_translations = channel->positions.size();
    output_channel.num_scales = channel->scales.size();
    output_channel.num_rotations = channel->rotations.size();
    outfile.write((char*)&output_channel, sizeof(RAnimationMeshFileChannel));

    std::vector<RAnimationMeshFileChannelRotation> out_rotations(channel->rotations.size());
    std::vector<RAnimationMeshFileChannelVector> out_translations(channel->positions.size());
    std::vector<RAnimationMeshFileChannelVector> out_scales(channel->scales.size());

    for (int i = 0; i < out_rotations.size(); ++i) {
        out_rotations[i].time_tick = channel->rotations[i].first;
        out_rotations[i].quat = channel->rotations[i].second;
    }

    for (int i = 0; i < out_translations.size(); ++i) {
        out_translations[i].time_tick = channel->positions[i].first;
        out_translations[i].vector = channel->positions[i].second;
    }

    for (int i = 0; i < out_scales.size(); ++i) {
        out_scales[i].time_tick = channel->scales[i].first;
        out_scales[i].vector = channel->scales[i].second;
    }

    outfile.write(
        (char*)out_scales.data(),
        out_scales.size() * sizeof(RAnimationMeshFileChannelVector)
    );
    outfile.write(
        (char*)out_rotations.data(),
        out_rotations.size() * sizeof(RAnimationMeshFileChannelRotation)
    );
    outfile.write(
        (char*)out_translations.data(),
        out_translations.size() * sizeof(RAnimationMeshFileChannelVector)
    );
}

static void WriteAnimationChannels(
    std::ofstream &outfile,
    const struct RSkinnedAnimation *animation)
{
    for (int i = 0; i < animation->GetChannelCount(); i++) {
        const RAnimationChannel *channel = animation->GetChannel(i);
        WriteAnimationChannel(outfile, channel);
    }
}

static void WriteAnimations(
    std::ofstream &outfile,
    const struct RSkinnedAnimation **animations,
    int num_animations)
{
    for (int i = 0; i < num_animations; ++i) {
        RAnimationMeshFileAnimation output_animation{};
        strcpy(output_animation.name, animations[i]->GetName().c_str());
        output_animation.num_channels = animations[i]->GetChannelCount();
        output_animation.ticks_per_second = animations[i]->GetTicksPerSec();
        output_animation.total_ticks = animations[i]->GetDurationTicks();
    }

    for (int i = 0; i < num_animations; ++i) {
        WriteAnimationChannels(outfile, animations[i]);
    }
}

void WriteAnimationMeshFile(
    const char *filename,
    const struct RSkinnedMesh *skinned_mesh,
    const struct RSkinnedAnimation **animations,
    int num_animations, int flags
)
{
    std::ofstream outfile(filename, std::ios::binary);
    RAnimationMeshFileHeader header{};
    header.root_transform = skinned_mesh->GetRootMatrix();
    header.compression_enabled = 0; // TODO later

    if (flags & kUseSkeleton) {
        header.bone_count = skinned_mesh->GetBoneCount();
    }
    if (flags & kUseMesh) {
        header.submesh_count = skinned_mesh->GetSubmeshCount();
    }
    if (flags & kUseAnimations) {
        header.animation_count = num_animations;
    }

    outfile.write((char*)&header, sizeof(RAnimationMeshFileHeader));

    if (flags & kUseSkeleton) {
        WriteBones(outfile, skinned_mesh);
    }

    if (flags & kUseMesh) {
        WriteSubmeshes(outfile, skinned_mesh);
    }

    if (flags & kUseAnimations) {
        WriteAnimations(outfile, animations, num_animations);
    }
}

}