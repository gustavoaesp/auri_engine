#include "skinned_animation.hpp"


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace eng
{

RAnimationChannel::RAnimationChannel(const RSkinnedMesh::Bone *_bone, const aiNodeAnim *channel):
    bone(_bone)
{
    positions.reserve(channel->mNumPositionKeys);
    scales.reserve(channel->mNumScalingKeys);
    rotations.reserve(channel->mNumRotationKeys);

    for (int i = 0; i < channel->mNumPositionKeys; ++i) {
        aiVectorKey *key = &channel->mPositionKeys[i];

        positions.push_back(
            std::pair<float, vec3f>(
                key->mTime,
                vec3f(
                    key->mValue.x,
                    key->mValue.y,
                    key->mValue.z
                )
            )
        );
    }

    for (int i = 0; i < channel->mNumScalingKeys; ++i) {
        aiVectorKey *key = &channel->mScalingKeys[i];

        scales.push_back(
            std::pair<float, vec3f>(
                key->mTime,
                vec3f(
                    key->mValue.x,
                    key->mValue.y,
                    key->mValue.z
                )
            )
        );
    }

    for (int i = 0; i < channel->mNumRotationKeys; ++i) {
        aiQuatKey *key = &channel->mRotationKeys[i];


        rotations.push_back(
            std::pair<float, Quaternion>(
                key->mTime,
                Quaternion(
                    key->mValue.x,
                    key->mValue.y,
                    key->mValue.z,
                    key->mValue.w
                ).Normalize()
            )
        );
    }
}

RSkinnedAnimation::RSkinnedAnimation(const RSkinnedMesh *skinned_mesh, const aiAnimation *ai_anim):
    duration_ticks_(ai_anim->mDuration),
    ticks_per_sec_(ai_anim->mTicksPerSecond)
{
    if (!ai_anim) {
        std::cerr << "aiAnimation is null!\n";
        return;
    }

    channels_.reserve(ai_anim->mNumChannels);
    for (int i = 0; i < ai_anim->mNumChannels; ++i) {
        aiNodeAnim *ai_channel = ai_anim->mChannels[i];
        const RSkinnedMesh::Bone *bone = skinned_mesh->GetBone(ai_channel->mNodeName.C_Str());
        if (!bone) {
            std::cerr << "Bone \"" << ai_channel->mNodeName.C_Str() << "\" not found!\n";
            continue;
        }
        channels_.push_back(
            std::make_unique<RAnimationChannel>(
                bone,
                ai_channel
            )
        );
    }
}

}