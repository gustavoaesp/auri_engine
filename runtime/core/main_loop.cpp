#include "core/main_loop.hpp"
#include "core/game_mode.hpp"
#include "core/global_context.hpp"

#include "input/input.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "backend_vulkan/backend.hpp"
#include "input/backends/glfw.hpp"
#include "render/renderer.hpp"


namespace eng
{

std::unique_ptr<GlobalContext> g_context;

CMainLoop::~CMainLoop()
{
    g_context->renderer->GetBackend()->Finalize();
    g_game_mode.reset();
    g_context.reset();
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void CMainLoop::Init()
{
    VkSurfaceKHR win_surface;
    if (!glfwInit()) {
        std::cerr << "Error initializing glfw\n";
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    std::unique_ptr<eng::VulkanInstance> vk_instance;
    vk_instance = std::make_unique<eng::VulkanInstance>(
        std::vector<const char*>{},
        std::vector<const char*>{"VK_LAYER_KHRONOS_validation"}
    );
    window_ = glfwCreateWindow(1600, 900, "Test", nullptr, nullptr);
    glfwCreateWindowSurface(vk_instance->get(), window_, nullptr, &win_surface);
    std::unique_ptr<eng::VulkanRenderBackend> backend = std::make_unique<eng::VulkanRenderBackend>(
        std::move(vk_instance),
        window_,
        win_surface
    );

    g_context = std::make_unique<GlobalContext>();
    g_context->input_manager = std::make_unique<eng::InputManager>(
        std::make_unique<eng::GLFWInput>(window_),
        "input.json"
    );
    backend->InitializeGUI();
    g_context->renderer = std::make_unique<eng::RRenderer>(
        std::move(backend)
    );

    if (g_game_mode) {
        g_game_mode->Init();
    }
}

int CMainLoop::Run()
{
    while (!glfwWindowShouldClose(window_)) {
        g_context->input_manager->Poll();
        g_context->renderer->BeginFrame();
        if (g_context->active_scene) {
            g_context->renderer->Render(*g_context->active_scene);
        }
        g_context->renderer->Present();

        g_game_mode->Tick(0.016f);
    }

    return 0;
}

}