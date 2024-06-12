#include "scene/scene_camera.hpp"

namespace eng
{

RSceneCamera::RSceneCamera(const vec3f& pos, const vec3f& look_pos, const vec3f& up_vector, float fovy):
    position(pos), look_pos(look_pos), up(up_vector), fovy(fovy)
{
}

}