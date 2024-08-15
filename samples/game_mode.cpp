#include "game_mode.hpp"

#include <core/global_context.hpp>
#include <input/input.hpp>
#include <render/managers/texture_manager.hpp>
#include <render/managers/mesh_manager.hpp>
#include <render/renderer.hpp>
#include <render/transform/matrix.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static eng::Vertex_NorTuv my_plane[4] = {
    {eng::vec3f(-1.0f, 0.0f, 1.0f), eng::vec3f(0.0f, 1.0f, 0.0f), eng::vec2f(0.0f, 8.0f)},
    {eng::vec3f( 1.0f, 0.0f, 1.0f), eng::vec3f(0.0f, 1.0f, 0.0f), eng::vec2f(8.0f, 8.0f)},
    {eng::vec3f( 1.0f, 0.0f,-1.0f), eng::vec3f(0.0f, 1.0f, 0.0f), eng::vec2f(8.0f, 0.0f)},
    {eng::vec3f(-1.0f, 0.0f,-1.0f), eng::vec3f(0.0f, 1.0f, 0.0f), eng::vec2f(0.0f, 0.0f)}
};

static uint32_t my_plane_indices[6] = {
    0, 1, 3, 1, 2, 3
};

static float hangle = 0, vangle = 0;

static void CameraSpheric(eng::RSceneCamera *cam, const eng::vec2f &mouse_surface)
{
    hangle += mouse_surface(0) * 0.005f;
    vangle -= mouse_surface(1) * 0.005f;

    if (vangle >= M_PI / 2) {
        vangle = M_PI / 2;
    } else if (vangle <= -M_PI / 2) {
        vangle = -M_PI / 2;
    }

    float length = 4.0f;
    cam->position(1) = sin(vangle);
    cam->position(0) = cos(hangle) * cos(vangle);
    cam->position(2) = sin(hangle) * cos(vangle);
    cam->position *= length;

    cam->position += cam->look_pos;
}

int MyGameMode::Init()
{
    int ret = CGameMode::Init();
    if (ret) { return ret; }

    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    const aiScene *ai_scene = importer.ReadFile(
        //"anim2.fbx",
        "link.gltf",
        //"huesitos.fbx",
        //"test.gltf",
        //"jog.fbx",
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PopulateArmatureData
    );

    std::shared_ptr<eng::RSkinnedMesh> skinned_mesh_base = std::make_shared<eng::RSkinnedMesh>(
        ai_scene
    );
    std::shared_ptr<eng::RSkinnedAnimation> skinned_animation = std::make_shared<eng::RSkinnedAnimation>(
        skinned_mesh_base.get(),
        ai_scene->mAnimations[0]
    );

    scene_ = std::make_shared<eng::RScene>();
    eng::g_context->active_scene = scene_;

    eng::RMaterial material;
    material.diffuse = eng::g_context->texture_manager->Get("textures/texture.tex");
    material.diffuse = eng::g_context->texture_manager->Get("textures/grass.tex");

    std::unique_ptr<eng::RSubmesh> plane_submesh = std::make_unique<eng::RSubmesh>(
        eng::g_context->renderer->GetBackend(),
        eng::RVertexType::kVertexPos3Nor3Tex2,
        my_plane, 4,
        my_plane_indices,
        6,
        std::move(material)
    );

    std::shared_ptr<eng::RMesh> plane_mesh(new eng::RMesh);
    plane_mesh->AddSubmesh(std::move(plane_submesh));
    eng::g_context->mesh_manager->Add("my_plane", std::move(plane_mesh));

    eng::vec3f mesh_pos(0.0f, 0.0f, 0.0f);
    eng::Quaternion mesh_rot(0, 0, 0, 0);
    eng::vec3f mesh_scale(1.0f, 1.0f, 1.0f);

    plane_ =
        std::make_shared<eng::RSceneMesh>(
            "my_plane",
            mesh_pos,
            mesh_rot,
            mesh_scale
        );

    tree_ =
        std::make_shared<eng::RSceneMesh>(
            "out.c3d",
            eng::vec3f(2.0f, 0.0f, 2.0f),
            eng::Quaternion(0, 0, 0, 0),
            eng::vec3f(0.2f, 0.2f, 0.2f)
        );

    skinned_ = std::make_shared<eng::RSceneSkinnedMesh>(
        skinned_mesh_base,
        skinned_animation,
        eng::vec3f(0.0f, 0.0f, 0.0f),
        eng::Quaternion(0, 0, 0, 0),
        eng::vec3f(1.0f, 1.0f, 1.0f)
        //eng::vec3f(0.001f,0.001f,0.001f)
    );

    scene_->active_camera = std::make_unique<eng::RSceneCamera>(
        eng::vec3f(1.0f, 1.0f, -1.0f),
        eng::vec3f(0.0f, 0.0f, 0.0f),
        eng::vec3f(0.0f, 1.0f, 0.0f),
        60.0f
    );

    std::shared_ptr<eng::RSceneLight> light = std::make_shared<eng::RSceneLight>();
    light->direction = eng::vec3f(1.0f, -0.0f, 0.0f);
    light->color = eng::vec3f(1.0f, 1.0f, 1.0f);

    std::shared_ptr<eng::RSceneLight> light2 = std::make_shared<eng::RSceneLight>();
    light2->direction = eng::vec3f(-1.0f, -0.0f, 0.0f);
    light2->color = eng::vec3f(0.0f, 0.0f, 1.0f);

    scene_->scene_meshes.push_back(plane_);
    scene_->scene_meshes.push_back(tree_);

    scene_->scene_skinned_meshes.push_back(skinned_);

    scene_->scene_lights.push_back(light);
    scene_->scene_lights.push_back(light2);

    scene_->ambient_color = eng::vec3f(0.4f, 0.4f, 0.4f);

    plane_->scale = eng::vec3f(4.0f, 4.0f, 4.0f);

    eng::g_context->input_manager->SetMouseTracking(true);

    return 0;
}

void MyGameMode::Tick(float dt)
{
    eng::vec3f diff_camera = scene_->active_camera->look_pos - scene_->active_camera->position;
    diff_camera(1) = 0;
    diff_camera = diff_camera.unit();

    skinned_->Tick(0.016f);

    if (eng::g_context->input_manager->GetState("up")) {
        scene_->active_camera->look_pos += diff_camera * ((float) 0.016f / 4.0f);
    }
    if (eng::g_context->input_manager->GetState("down")) {
        scene_->active_camera->look_pos -= diff_camera * ((float) 0.016f / 4.0f);
    }
    if (eng::g_context->input_manager->GetState("left")) {
        scene_->active_camera->look_pos += eng::vec3f(
            -diff_camera(2),
            0.0f,
            diff_camera(0)
        ) * ((float) 0.016f / 4.0f);
    }
    if (eng::g_context->input_manager->GetState("right")) {
        scene_->active_camera->look_pos -= eng::vec3f(
            -diff_camera(2),
            0.0f,
            diff_camera(0)
        ) * ((float) 0.016f / 4.0f);
    }

    CameraSpheric(scene_->active_camera.get(), eng::g_context->input_manager->GetSurface("camera"));
}