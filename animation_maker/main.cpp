
#include "game_mode.hpp"

#include <core/main_loop.hpp>

int main(int argc, char **argv)
{
    eng::CMainLoop main_loop;

    main_loop.Init();
    main_loop.Run();

    return 0;
}