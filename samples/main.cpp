#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <backend_vulkan/instance.hpp>
#include <backend_vulkan/backend.hpp>
#include <render/renderer.hpp>
#include <render/scene/scene.hpp>
#include <render/transform/vector.hpp>
#include <input/input.hpp>
#include <input/backends/glfw.hpp>

eng::Vertex_NorTuv my_triangle[3] = {
    {eng::vec3f( 0.0f, 1.0f, 0.0f), eng::vec3f(0.0f, 0.0f, -1.0f), eng::vec2f(0.5f, 1.0f)},
    {eng::vec3f( 1.0f,-1.0f, 0.0f), eng::vec3f(0.0f, 0.0f, -1.0f), eng::vec2f(1.0f, 0.0f)},
    {eng::vec3f(-1.0f,-1.0f, 0.0f), eng::vec3f(0.0f, 0.0f, -1.0f), eng::vec2f(0.0f, 0.0f)}
};

uint32_t my_triangle_indices[3] = {
    0, 1, 2
};

eng::Vertex_NorTuv my_plane[4] = {
    {eng::vec3f(-1.0f, 0.0f, 1.0f), eng::vec3f(0.0f, 1.0f, 0.0f), eng::vec2f(0.0f, 4.0f)},
    {eng::vec3f( 1.0f, 0.0f, 1.0f), eng::vec3f(0.0f, 1.0f, 0.0f), eng::vec2f(4.0f, 4.0f)},
    {eng::vec3f( 1.0f, 0.0f,-1.0f), eng::vec3f(0.0f, 1.0f, 0.0f), eng::vec2f(4.0f, 0.0f)},
    {eng::vec3f(-1.0f, 0.0f,-1.0f), eng::vec3f(0.0f, 1.0f, 0.0f), eng::vec2f(0.0f, 0.0f)}
};

uint32_t my_plane_indices[6] = {
    0, 1, 3, 1, 2, 3
};

eng::RBlendState blend_state{};

struct UniformContents
{
    eng::mtx4f mvp;
};

static float hangle = 0;
static float vangle = M_PI / 4.0f;

void CameraSpheric(eng::RSceneCamera *cam, const eng::vec2f &mouse_surface)
{
    hangle += mouse_surface(0) * 0.016f / 4.0f;
    vangle += mouse_surface(1) * 0.016f / 4.0f * -1.0f;

    float length = 3.0f;
    cam->position(1) = sin(vangle);
    cam->position(0) = cos(hangle) * cos(vangle);
    cam->position(2) = sin(hangle) * cos(vangle);
    cam->position *= length;

    cam->position += cam->look_pos;

}

int main(int argc, char** argv)
{
    if (!glfwInit()) {
        std::cerr << "Error initializing glfw\n";
        return -1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    std::unique_ptr<eng::VulkanInstance> vk_instance;
    vk_instance = std::make_unique<eng::VulkanInstance>(
        std::vector<const char*>{},
        std::vector<const char*>{"VK_LAYER_KHRONOS_validation"}
    );

    eng::RScene scene;

    {
        VkSurfaceKHR win_surface;
        GLFWwindow *window = glfwCreateWindow(1600, 900, "Test", nullptr, nullptr);
        glfwCreateWindowSurface(vk_instance->get(), window, nullptr, &win_surface);
        std::unique_ptr<eng::VulkanRenderBackend> backend = std::make_unique<eng::VulkanRenderBackend>(
            std::move(vk_instance),
            window,
            win_surface
        );
        eng::g_input_manager = std::make_unique<eng::InputManager>(
            std::make_unique<eng::GLFWInput>(window),
            "input.json"
        );
        backend->InitializeGUI();
        {
            std::unique_ptr<eng::RRenderer> renderer = std::make_unique<eng::RRenderer>(
                std::move(backend)
            );
            eng::RMaterial material;
            material.diffuse = eng::g_texture_manager->Get("textures/texture.tex");
            std::unique_ptr<eng::RSubmesh> submesh = std::make_unique<eng::RSubmesh>(
                renderer->GetBackend(),
                eng::RVertexType::kVertexPos3Nor3Tex2,
                my_triangle, 3,
                my_triangle_indices, 3,
                std::move(material)
            );
            std::shared_ptr<eng::RMesh> my_mesh =
                std::shared_ptr<eng::RMesh>(new eng::RMesh);
            my_mesh->AddSubmesh(std::move(submesh));

            material.diffuse = eng::g_texture_manager->Get("textures/grass.tex");
            std::unique_ptr<eng::RSubmesh> plane_submesh = std::make_unique<eng::RSubmesh>(
                renderer->GetBackend(),
                eng::RVertexType::kVertexPos3Nor3Tex2,
                my_plane, 4,
                my_plane_indices,
                6,
                std::move(material)
            );
            std::shared_ptr<eng::RMesh> plane_mesh(new eng::RMesh);
            plane_mesh->AddSubmesh(std::move(plane_submesh));

            eng::g_mesh_manager->Add("my_mesh", std::move(my_mesh));
            eng::g_mesh_manager->Add("my_plane", std::move(plane_mesh));

            eng::vec3f mesh_pos(0.0f, 0.0f, 0.0f);
            eng::Quaternion mesh_rot(0, 0, 0, 0);
            eng::vec3f mesh_scale(1.0f, 1.0f, 1.0f);

            std::shared_ptr<eng::RSceneMesh> mesh_instance =
                std::make_shared<eng::RSceneMesh>(
                    "my_mesh",
                    mesh_pos,
                    mesh_rot,
                    mesh_scale
                );

                mesh_instance->position(0) = -4.0f;
            std::shared_ptr<eng::RSceneMesh> plane_instance =
                std::make_shared<eng::RSceneMesh>(
                    "my_plane",
                    mesh_pos,
                    mesh_rot,
                    mesh_scale
                );

            scene.active_camera = std::make_unique<eng::RSceneCamera>(
                eng::vec3f(1.0f, 1.0f, -1.0f),
                eng::vec3f(0.0f, 0.0f, 0.0f),
                eng::vec3f(0.0f, 1.0f, 0.0f),
                60.0f
            );

            scene.scene_meshes.push_back(plane_instance);
            scene.scene_meshes.push_back(mesh_instance);

            mesh_instance->scale(0) = 0.8f;

            plane_instance->scale = eng::vec3f(4.0f, 4.0f, 4.0f);
            eng::g_input_manager->SetMouseTracking(true);

            float angle = 0;
            while (!glfwWindowShouldClose(window)) {
                renderer->BeginFrame();
                renderer->Render(scene);
                renderer->Present();
                eng::g_input_manager->Poll();

                mesh_instance->rotation.setAxisRotation(eng::vec3f(0.0f,0.0f, 1.0f), angle);
                angle += 0.01f;

                eng::vec3f diff_camera = scene.active_camera->look_pos - scene.active_camera->position;
                diff_camera(1) = 0;
                diff_camera = diff_camera.unit();

                if (eng::g_input_manager->GetState("up")) {
                    scene.active_camera->look_pos += diff_camera * ((float) 0.016f / 4.0f);
                }
                if (eng::g_input_manager->GetState("down")) {
                    mesh_instance->position(2) -= 0.01f;
                }
                if (eng::g_input_manager->GetState("left")) {
                    mesh_instance->position(0) -= 0.01f;
                }
                if (eng::g_input_manager->GetState("right")) {
                    mesh_instance->position(0) += 0.01f;
                }

                CameraSpheric(scene.active_camera.get(), eng::g_input_manager->GetSurface("camera"));
            }
        }
        glfwDestroyWindow(window);
    }

    glfwTerminate();
    return 0;
}