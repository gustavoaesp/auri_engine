#include "scene/scene_skinned_mesh.hpp"

#include "transform/matrix.hpp"

namespace eng
{

RSceneSkinnedMesh::RSceneSkinnedMesh(
    std::shared_ptr<RSkinnedMesh> &skinned_mesh,
    std::shared_ptr<RSkinnedAnimation> &skinned_animation,
    const vec3f &_pos, const Quaternion &_rot,
    const vec3f &_scale
): position(_pos), rotation(_rot), scale(_scale)
{
    mesh = skinned_mesh;
    bones = std::vector<mtx4f>(mesh->GetBoneCount());
    animation = skinned_animation;
    current_time = 0;
}

static mtx4f ComputeTransform(
    float curr_time,
    float anim_duration_ticks,
    float anim_ticks_per_sec,
    const RAnimationChannel *channel)
{
    auto vector_find = [](float current_tick, const std::vector<std::pair<float, vec3f>> &channel_array) {
        for (int i = 0; i < channel_array.size() - 1; ++i) {
            const auto &current_position = channel_array[i];
            const auto &next_position = channel_array[i + 1];
            if (!(current_tick >= current_position.first && current_tick <= next_position.first)) {
                continue;
            }

            float t = (current_tick - current_position.first) / (next_position.first - current_position.first);
            return vec3f::Lerp(current_position.second, next_position.second, t);
        }

        return channel_array[0].second;
    };
    auto rotation_find = [](float current_tick, const std::vector<std::pair<float, Quaternion>>& channel_array) {
        for (int i = 0; i < channel_array.size() - 1; ++i) {
            const auto &current_position = channel_array[i];
            const auto &next_position = channel_array[i + 1];
            if (!(current_tick >= current_position.first && current_tick <= next_position.first)) {
                continue;
            }

            float t = (current_tick - current_position.first) / (next_position.first - current_position.first);
            return Quaternion::Lerp(current_position.second, next_position.second, t);
        }

        return channel_array[0].second;
    };

    float current_tick = curr_time * anim_ticks_per_sec;
    vec3f position, scale;
    Quaternion rotation(0, 0, 0, 0);

    position = (channel->positions.size() > 1) ?
        vector_find(current_tick, channel->positions) : channel->positions[0].second;
    scale = (channel->scales.size() > 1) ?
        vector_find(current_tick, channel->scales) : channel->scales[0].second;
    rotation = (channel->rotations.size() > 1) ?
        rotation_find(current_tick, channel->rotations) : channel->rotations[0].second;

    mtx4f transform = CreateScaleMatrix(
        scale(0),
        scale(1),
        scale(2)
    );
    transform *= rotation.Normalize().toMatrix();
    transform *= CreateTranslateMatrix(position);

    return transform;
}

void RSceneSkinnedMesh::Tick(float dt)
{
    const int num_bones = mesh->GetBoneCount();

    for (int i = 0; i < num_bones; ++i) {
        bones[i] = mesh->GetBone(i)->local_mtx;
    }

    for (int c = 0; c < animation->GetChannelCount(); ++c) {
        const RAnimationChannel *channel = animation->GetChannel(c);
        int bone_idx = mesh->GetBoneIndex(channel->bone->name);
        if (bone_idx < 0) {
            std::cerr << "Channel bone " << channel->bone->name << " not found\n";
            continue;
        }
        mtx4f transform = ComputeTransform(
            current_time,
            animation->GetDurationTicks(),
            animation->GetTicksPerSec(),
            channel
        );
        bones[bone_idx] = (bone_idx) ? transform : (transform * mesh->GetRootMatrix());
    }

    for (int i = 0; i < num_bones; ++i) {
        const RSkinnedMesh::Bone *mesh_bone = mesh->GetBone(i);

        if (mesh_bone->parent_idx < 0) {
            continue;
        }

        bones[i] = bones[i] * bones[mesh_bone->parent_idx];
    }

    for (int i = 0; i < num_bones; ++i) {
        const RSkinnedMesh::Bone *mesh_bone = mesh->GetBone(i);
        bones[i] = mesh_bone->offset_mtx * bones[i];
    }

    current_time += (dt / 1);
    float animation_duration_secs = animation->GetDurationTicks() / animation->GetTicksPerSec();
    if (current_time >= animation_duration_secs) {
        current_time -= animation_duration_secs;
    }
}

}