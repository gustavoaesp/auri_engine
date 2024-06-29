#ifndef _CORE_GLOBAL_CONTEXT_HPP_
#define _CORE_GLOBAL_CONTEXT_HPP_

#include <memory>

namespace eng
{

class CGameMode;
class InputManager;
class RMeshManager;
class RRenderer;
struct RScene;
class RShaderList;
class RTextureManager;

struct GlobalContext
{
    std::unique_ptr<RRenderer> renderer;
    std::unique_ptr<RMeshManager> mesh_manager;
    std::unique_ptr<RTextureManager> texture_manager;
    std::unique_ptr<RShaderList> shader_list;
    std::unique_ptr<InputManager> input_manager;

    uint32_t frame_width, frame_height;

    std::shared_ptr<RScene> active_scene;
};

extern std::unique_ptr<GlobalContext> g_context;
extern std::unique_ptr<CGameMode> g_game_mode;

}

#endif