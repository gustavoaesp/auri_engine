#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <backend_vulkan/instance.hpp>
#include <backend_vulkan/backend.hpp>
#include <render/renderer.hpp>
#include <render/scene/scene.hpp>

/*eng::Vertex my_triangle[3] = {
    {eng::vec3f( 0.0f, 1.0f, 0.0f), eng::vec4f(0.5f, 1.0f, 0.0f, 1.0f)},
    {eng::vec3f( 1.0f,-1.0f, 0.0f), eng::vec4f(1.0f, 0.0f, 0.0f, 1.0f)},
    {eng::vec3f(-1.0f,-1.0f, 0.0f), eng::vec4f(0.0f, 0.0f, 1.0f, 1.0f)}
};*/

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
            std::shared_ptr<eng::RSceneMesh> plane_instance =
                std::make_shared<eng::RSceneMesh>(
                    "my_plane",
                    mesh_pos,
                    mesh_rot,
                    mesh_scale
                );

            scene.scene_meshes.push_back(plane_instance);
            scene.scene_meshes.push_back(mesh_instance);

            plane_instance->scale = eng::vec3f(4.0f, 4.0f, 4.0f);

            float angle = 0;
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                renderer->BeginFrame();
                renderer->Render(scene);
                renderer->Present();
                mesh_instance->rotation.setAxisRotation(eng::vec3f(0.0f,0.0f, 1.0f), angle);
                angle += 0.01f;
            }
        }
        glfwDestroyWindow(window);
    }

    glfwTerminate();
    return 0;
}