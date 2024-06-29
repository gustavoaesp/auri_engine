#ifndef _GAME_MODE_HPP_
#define _GAME_MODE_HPP_

#include <core/game_mode.hpp>
#include <render/scene/scene_light.hpp>
#include <render/scene/scene_mesh.hpp>
#include <render/scene/scene.hpp>

#include <memory>

class MyGameMode : public eng::CGameMode
{
public:
    MyGameMode() {}

    int Init() override;
    void Tick(float dt) override;

private:
    std::shared_ptr<eng::RSceneLight> light_;
    std::shared_ptr<eng::RSceneLight> light2_;

    std::shared_ptr<eng::RSceneMesh> tree_;
    std::shared_ptr<eng::RSceneMesh> plane_;

    std::shared_ptr<eng::RScene> scene_;
};

#endif