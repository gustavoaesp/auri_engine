#ifndef _RENDER_SKINNED_ANIMATION_HPP_
#define _RENDER_SKINNED_ANIMATION_HPP_

#include <math/quaternion.hpp>

#include "skinned_mesh.hpp"

class aiAnimation;
class aiNodeAnim;

namespace eng
{

struct RAnimationChannel
{
    RAnimationChannel(const RSkinnedMesh::Bone *bone, const aiNodeAnim *channel);
    const RSkinnedMesh::Bone *bone;
    std::vector<std::pair<float, vec3f>> positions;
    std::vector<std::pair<float, vec3f>> scales;
    std::vector<std::pair<float, Quaternion>> rotations;
};

class RSkinnedAnimation
{
public:
    RSkinnedAnimation(const RSkinnedMesh*, const aiAnimation *);

    int GetChannelCount() const {return channels_.size();}

    const RAnimationChannel *GetChannel(int idx) const {
        if (idx > channels_.size()) return nullptr;
        return channels_[idx].get();
    }

    float GetDurationTicks() const { return duration_ticks_; }
    float GetTicksPerSec() const { return ticks_per_sec_; }

    const std::string &GetName() const { return name_; }

private:
    std::vector<std::unique_ptr<RAnimationChannel>> channels_;
    float duration_ticks_;
    float ticks_per_sec_;

    std::string name_;
};

}

#endif