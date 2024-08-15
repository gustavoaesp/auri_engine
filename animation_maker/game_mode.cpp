#include "game_mode.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <common/stb_image.h>

#include <core/global_context.hpp>
#include <render/managers/texture_manager.hpp>
#include <render/renderer.hpp>
#include <render/scene/scene.hpp>
#include <render/scene/scene_skinned_mesh.hpp>
#include <render/scene/scene_mesh.hpp>
#include <render/anim_mesh_file.hpp>

#include <filesystem>
#include <memory>
#include <string>

#include <third_party/imgui/imgui.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static float hangle = 0;
static float vangle = 0;
static float distance = 4.0f;


static eng::RTextureFile ReadImageFile(const char *filename)
{
    eng::RTextureFile texture_file;
    uint8_t *pixels = stbi_load(
        filename,
        &texture_file.header.width,
        &texture_file.header.height,
        &texture_file.header.channels,
    4);

    if (!pixels) {
        return eng::RTextureFile{};
    }

    texture_file.data = std::vector<uint8_t>(
        texture_file.header.width
        * texture_file.header.height
        * texture_file.header.channels
    );

    memcpy(
        texture_file.data.data(),
        pixels,
        texture_file.data.size()
    );

    return std::move(texture_file);
}

std::shared_ptr<eng::RTexture> LoadTexture(const char *filename)
{
    eng::RTextureFile file = ReadImageFile(filename);

    if (!file.header.width || !file.header.height) {
        return nullptr;
    }

    return std::shared_ptr<eng::RTexture>(
        eng::g_context->renderer->GetBackend()->CreateTexture2D(&file)
    );
}

void CameraSpheric()
{
    eng::RSceneCamera *cam = eng::g_context->active_scene->active_camera.get();
    if (vangle >= M_PI / 2) {
        vangle = M_PI / 2;
    } else if (vangle <= -M_PI / 2) {
        vangle = -M_PI / 2;
    }

    cam->position(1) = sin(vangle);
    cam->position(0) = cos(hangle) * cos(vangle);
    cam->position(2) = sin(hangle) * cos(vangle);
    cam->position *= distance;

    cam->position += cam->look_pos;
}

namespace eng
{
    std::unique_ptr<CGameMode> g_game_mode = std::make_unique<AnimationGameMode>();
}

int AnimationGameMode::Init()
{
    eng::g_context->active_scene = std::make_shared<eng::RScene>();
    eng::g_context->active_scene->active_camera = std::make_shared<eng::RSceneCamera>(
        eng::vec3f(6.0f, 5.0f, 6.0f),
        eng::vec3f(0.0f, 0.0f, 0.0f),
        eng::vec3f(0.0f, 1.0f, 0.0f),
        60.0f
    );
    std::shared_ptr<eng::RSceneLight> light = std::make_shared<eng::RSceneLight>();
    light->type = eng::RSceneLightType::kLightDirectional;
    light->direction = eng::vec3f(1.0f, -1.0f, 1.0f);
    light->color = eng::vec3f(1.0f, 1.0f, 1.0f);

    std::shared_ptr<eng::RSceneLight> light2 = std::make_shared<eng::RSceneLight>();
    light2->type = eng::RSceneLightType::kLightDirectional;
    light2->direction = eng::vec3f(-1.0f, -1.0f, -1.0f);
    light2->color = eng::vec3f(1.0f, 1.0f, 1.0f);

    eng::g_context->active_scene->ambient_color = eng::vec3f(0.8f, 0.8f, 0.8f);
    eng::g_context->active_scene->scene_lights.push_back(light);
    eng::g_context->active_scene->scene_lights.push_back(light2);

    auto mesh_test = std::make_shared<eng::RSceneMesh>(
        "out.c3d",
        eng::vec3f(2.0f, 0.0f, 2.0f),
        eng::Quaternion(0, 0, 0, 0),
        eng::vec3f(0.2f, 0.2f, 0.2f)
    );

    eng::g_context->active_scene->scene_meshes.push_back(mesh_test);
    return 0;
}

void AnimationGameMode::Tick(float dt)
{
    ImGui::Begin("Import");
        ImGui::InputText("File name: ", filename_, 256);
        if(ImGui::Button("Replace all!")) {
            LoadSkinnedMesh(filename_);
        }
        if(ImGui::Button("Load Animations Only")) {
        }
        if(ImGui::Button("Load Skeleton only (Replaces everything!)")) {
        }
        if(ImGui::Button("Load Mesh")) {
        }
    ImGui::End();

    ImGui::Begin("Scene control");
        ImGui::SliderAngle("Horizontal", &hangle);
        ImGui::SliderAngle("Vertical", &vangle);
        ImGui::SliderFloat("Distance", &distance, 1.0f, 300.0f);
    ImGui::End();

    for (auto& skinned_mesh : eng::g_context->active_scene->scene_skinned_meshes) {
        skinned_mesh->Tick(0.016f);
    }

    if (skinned_mesh_) {
        ImGui::Begin("Mesh data");
            for (int i = 0; i < skinned_mesh_->GetSubmeshCount(); ++i) {
                eng::RSkinnedSubmesh *submesh = skinned_mesh_->GetSubmesh(i);
                if(ImGui::CollapsingHeader(submesh->name.c_str())) {
                    ImGui::InputText("New texture", submesh_dictionary_[submesh->name].texture_name_input, 256);
                    if (ImGui::Button("Load")) {
                        const char *input_name = submesh_dictionary_[submesh->name].texture_name_input;
                        auto texture = LoadTexture(input_name);
                        if (texture) {
                            submesh->material.diffuse = texture;
                            eng::g_context->texture_manager->Add(input_name, std::move(texture));
                        }
                    }
                }
            }
        ImGui::End();
    }

    ImGui::Begin("Export");
        ImGui::Checkbox("Export mesh", &export_mesh_);
        ImGui::Checkbox("Export skeleton", &export_skeleton_);
        ImGui::Checkbox("ExportAnimations", &export_animations_);
        ImGui::InputText("Base name on exports", export_base_name_, 256);
        if (ImGui::Button("Export")) {
            if (skinned_mesh_ && animations_.size()) {
                int write_flags = 0;
                write_flags |= (export_animations_) ?
                    eng::RSkinnedAnimationFileFlags::kUseAnimations : 0;
                write_flags |= (export_skeleton_) ?
                    eng::RSkinnedAnimationFileFlags::kUseSkeleton : 0;
                write_flags |= (export_mesh_) ?
                    eng::RSkinnedAnimationFileFlags::kUseMesh : 0;
                std::vector<const eng::RSkinnedAnimation*> animation_pointers;
                for (const auto &animation : animations_) {
                    animation_pointers.push_back(animation.get());
                }
                std::filesystem::create_directories(std::string("_export_") + export_base_name_);
                eng::WriteAnimationMeshFile(
                    (std::string("_export_")
                    + export_base_name_
                    + "/" + export_base_name_).c_str(),
                    skinned_mesh_.get(),
                    animation_pointers.data(), animation_pointers.size(),
                    write_flags
                );
            }
        }
    ImGui::End();

    CameraSpheric();
}

void AnimationGameMode::LoadSkinnedMesh(const char *filename)
{
    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    const aiScene *ai_scene = importer.ReadFile(
        filename,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PopulateArmatureData
    );

    if (!ai_scene) {
        std::cout << "Could not load model \"" << filename << "\"\n";
        return;
    }

    eng::g_context->active_scene->scene_skinned_meshes = {};
    skinned_mesh_.reset();
    animations_ = {};
    submesh_dictionary_ = {};

    skinned_mesh_ = std::make_shared<eng::RSkinnedMesh>(
        ai_scene
    );
    for (int i = 0; i < ai_scene->mNumAnimations; ++i) {
        animations_.push_back(
            std::make_shared<eng::RSkinnedAnimation>(
                skinned_mesh_.get(),
                ai_scene->mAnimations[i]
            )
        );
    }

    eng::g_context->active_scene->scene_skinned_meshes.push_back(
        std::make_shared<eng::RSceneSkinnedMesh>(
            skinned_mesh_,
            animations_[0],
            eng::vec3f(0.0f, 0.0f, 0.0f),
            eng::Quaternion(0, 0, 0, 0),
            eng::vec3f(1.0f, 1.0f, 1.0f)
        )
    );

    for (int i = 0; i < skinned_mesh_->GetSubmeshCount(); ++i) {
        eng::RSkinnedSubmesh *submesh = skinned_mesh_->GetSubmesh(i);

        submesh_dictionary_[submesh->name] = SkinnedSubmeshEdit{
            .current_texture = "", // TODO include the texture the submesh may want to load
            .texture_name_input = "",
            .submesh = submesh
        };
    }
}