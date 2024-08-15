#ifndef _ANIMATION_GAME_MODE_HPP_
#define _ANIMATION_GAME_MODE_HPP_

#include <core/game_mode.hpp>

#include <render/skinned_mesh.hpp>
#include <render/skinned_animation.hpp>

#include <memory>
#include <vector>

struct SkinnedSubmeshEdit
{
    std::string current_texture;
    char texture_name_input[256];
    eng::RSkinnedSubmesh *submesh;
};

class AnimationGameMode : public eng::CGameMode
{
public:
    AnimationGameMode() {}

    int Init() override;
    void Tick(float dt);

private:
    std::shared_ptr<eng::RSkinnedMesh> skinned_mesh_;
    std::vector<std::shared_ptr<eng::RSkinnedAnimation>> animations_;

    char filename_[256];

    void LoadSkinnedMesh(const char *filename);

    std::unordered_map<std::string, SkinnedSubmeshEdit> submesh_dictionary_;

    bool export_mesh_;
    bool export_skeleton_;
    bool export_animations_;

    char export_base_name_[256];
};

#endif