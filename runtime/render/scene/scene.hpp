#ifndef _RENDER_SCENE_HPP_
#define _RENDER_SCENE_HPP_

#include "scene/scene_mesh.hpp"

#include <memory>
#include <vector>

namespace eng
{

struct RScene
{
    std::vector<std::shared_ptr<RSceneMesh>> scene_meshes;
};

}

#endif