#ifndef _CORE_MAIN_LOOP_HPP_
#define _CORE_MAIN_LOOP_HPP_

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace eng
{

class CMainLoop
{
public:
    ~CMainLoop();
    void Init();

    int Run();

protected:
    GLFWwindow *window_;
};

}

#endif