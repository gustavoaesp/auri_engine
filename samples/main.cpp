#include <iostream>

#include "game_mode.hpp"
#include <core/main_loop.hpp>
#include <core/global_context.hpp>

namespace eng
{
    std::unique_ptr<CGameMode> g_game_mode = std::make_unique<MyGameMode>();
}

int main(int argc, char** argv)
{
    eng::CMainLoop main_loop;

    main_loop.Init();

    main_loop.Run();

    return 0;
}