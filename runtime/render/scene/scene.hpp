#ifndef _RENDER_SCENE_HPP_
#define _RENDER_SCENE_HPP_

#include "scene/scene_mesh.hpp"
#include "scene/scene_camera.hpp"
#include "scene/scene_light.hpp"

#include <memory>
#include <vector>

namespace eng
{

struct RScene
{
    std::vector<std::shared_ptr<RSceneMesh>> scene_meshes;

    std::shared_ptr<RSceneCamera> active_camera;

    std::vector<std::shared_ptr<RSceneLight>> scene_lights;

    vec3f ambient_color;
};

}

#endif