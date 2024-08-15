#include "skinned_mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <core/global_context.hpp>
#include <renderer.hpp>

#include "render/transform/matrix.hpp"

#include <common/stb_image.h>

namespace eng
{

struct BoneNode
{
    BoneNode *parent;
    std::vector<std::unique_ptr<BoneNode>> children;
    std::string name;
    eng::mtx4f transformation; // relative to parent!
    eng::mtx4f offset;
};

static void BuildBoneTree(
    std::unique_ptr<BoneNode> &root_bone,
    const std::unordered_map<std::string, aiBone*> &bones_dict,
    aiNode *node, BoneNode *current_parent)
{
    const auto &find_iter = bones_dict.find(node->mName.C_Str());

    if (find_iter != bones_dict.end()) {
        if (current_parent) {
            current_parent->children.push_back(std::make_unique<BoneNode>());
            current_parent->children.back()->name = node->mName.C_Str();
            current_parent->children.back()->parent = current_parent;
            current_parent = current_parent->children.back().get();

        } else {
            // we found the root probably!
            root_bone = std::make_unique<BoneNode>();
            root_bone->name = node->mName.C_Str();
            root_bone->parent = nullptr;
            current_parent = root_bone.get();
        }
        memcpy(
            &current_parent->transformation,
            &node->mTransformation,
            sizeof(eng::mtx4f)
        );
        memcpy(
            &current_parent->offset,
            &find_iter->second->mOffsetMatrix,
            sizeof(eng::mtx4f)
        );
        current_parent->offset = transpose(current_parent->offset);
        current_parent->transformation = transpose(current_parent->transformation);
    }

    for (int i = 0; i < node->mNumChildren; ++i) {
        BuildBoneTree(root_bone, bones_dict, node->mChildren[i], current_parent);
    }
}

static void BuildDictionary(
    std::unordered_map<std::string, aiBone*> &dict,
    aiMesh *mesh)
{
    for (int i = 0; i < mesh->mNumBones; ++i) {
        dict[mesh->mBones[i]->mName.C_Str()] = mesh->mBones[i];
    }
}

static void print_spaces(int num) {
    for (int i = 0; i < num; ++i) {
        std::cout << "\t";
    }
}

static void PrintNodeTree(const BoneNode *tree, int level) {
    print_spaces(level);
    std::cerr << "Node: " << tree->name << "\n";
    for (const auto& child : tree->children) {
        PrintNodeTree(child.get(), level + 1);
    }
}

static void LoadSubmeshTexture(RSkinnedSubmesh &submesh, const aiMaterial *material, const aiScene *ai_scene)
{
    aiString texture_filename;
    if (!material->GetTextureCount(aiTextureType_DIFFUSE)) {
        return;
    }
    material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_filename);

    // An '*' at the start of the string means the texture is embedded
    if (texture_filename.C_Str()[0] == '*') {
        int texture_index = std::atoi(texture_filename.C_Str() + 1);
        aiTexture *texture = ai_scene->mTextures[texture_index];

        if (!texture->mHeight) {
            RTextureFile file{};
            uint8_t *pixels = stbi_load_from_memory(
                (uint8_t*)texture->pcData,
                texture->mWidth,
                &file.header.width, &file.header.height, &file.header.channels, 4
            );
            file.data = std::vector<uint8_t>(file.header.width * file.header.height * file.header.channels);
            memcpy(file.data.data(), pixels, file.header.width * file.header.height * file.header.channels);
            submesh.material.diffuse = std::shared_ptr<RTexture>(
                g_context->renderer->GetBackend()->CreateTexture2D(
                    &file
                )
            );
            stbi_image_free(pixels);
        }
    }
}

RSkinnedMesh::RSkinnedMesh(const aiScene *scene)
{
    std::unordered_map<std::string, aiBone *> bone_dictionary;
    std::unique_ptr<BoneNode> bone_tree;

    if (!scene->mNumMeshes) {
        std::cerr << "Error: No meshes!\n";
        return;
    }

    // Here we are asumming we use the same skeleton for
    // all submeshes.
    // Weird shit will not be supported
    BuildDictionary(bone_dictionary, scene->mMeshes[0]);
    BuildBoneTree(bone_tree, bone_dictionary, scene->mRootNode, nullptr);
    BuildBoneArray(bone_tree.get());

    memcpy(
        &root_mtx_,
        &scene->mRootNode->mTransformation,
        sizeof(mtx4f)
    );
    root_mtx_ = transpose(root_mtx_);
    bones_[0].local_mtx = bones_[0].local_mtx * root_mtx_;

    PrintNodeTree(bone_tree.get(), 0);

    for (int i = 0; i < scene->mNumMeshes; ++i) {
        BuildSubmesh(scene->mMeshes[i]);
        LoadSubmeshTexture(
            submeshes_.back(),
            scene->mMaterials[scene->mMeshes[i]->mMaterialIndex],
            scene
        );
    }

    for (const auto &submesh : submeshes_) {
        for (const auto &vertex : submesh.vertices) {
            for (int i = 0; i < 4; ++i) {
                if (vertex.bones[i] >= bones_.size()) {
                    std::cerr << "Found vertex with bone > " << bones_.size() << "\n";
                }
            }
        }
    }
}

void RSkinnedMesh::BuildBoneArray(const BoneNode *root)
{
    bones_.push_back(Bone{});
    Bone &bone = bones_.back();

    if (root->parent) {
        bone.parent_idx = BoneLookupNameIndex(root->parent->name);
    } else {
        bone.parent_idx = -1;
    }

    bone.name = root->name;
    bone.local_mtx = root->transformation;
    bone.offset_mtx = root->offset;

    for (const auto &child :root->children) {
        BuildBoneArray(child.get());
    }

}

int32_t RSkinnedMesh::BoneLookupNameIndex(const std::string &name) const
{
    for (int i = 0; i < bones_.size(); ++i) {
        if (bones_[i].name != name) {
            continue;
        }

        return i;
    }

    return -1;
}

void RSkinnedMesh::BuildSubmesh(const aiMesh *mesh)
{
    RSkinnedSubmesh submesh;
    submesh.material.diffuse = g_context->texture_manager->Get("textures/texture.tex");
    submesh.vertices.reserve( mesh->mNumVertices );
    submesh.indices.reserve(mesh->mNumFaces * 3);

    for (int i = 0; i < mesh->mNumVertices; ++i) {
        submesh.vertices.push_back(
            eng::Vertex_NorTuv_Skinned{
                .position = eng::vec3f(
                    mesh->mVertices[i].x,
                    mesh->mVertices[i].y,
                    mesh->mVertices[i].z
                ),
                .normal = eng::vec3f(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                ),
                .texel = eng::vec2f(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                ),
                .bones = {},
                .weights = {}
            }
        );
    }

    for (int i = 0; i < mesh->mNumBones; ++i) {
        BuildWeightsFromBone(&submesh, mesh->mBones[i]);
    }

    for (int f = 0; f < mesh->mNumFaces; ++f) {
        const aiFace *face = &mesh->mFaces[f];
        if (face->mNumIndices != 3) {
            std::cerr << "face.indices != 3!\n";
        }
        for (int i = 0; i < face->mNumIndices; ++i) {
            submesh.indices.push_back(face->mIndices[i]);
        }
    }

    submesh.vertex_buffer = std::unique_ptr<eng::RBuffer>(
        eng::g_context->renderer->GetBackend()->CreateBuffer(
            eng::RBufferUsage::kVertex,
            sizeof(eng::Vertex_NorTuv_Skinned) * submesh.vertices.size(),
            submesh.vertices.data()
        )
    );
    submesh.index_buffer = std::unique_ptr<RBuffer>(
        eng::g_context->renderer->GetBackend()->CreateBuffer(
            RBufferUsage::kIndex,
            sizeof(uint32_t) * submesh.indices.size(),
            submesh.indices.data()
        )
    );

    submesh.name = mesh->mName.C_Str();

    submeshes_.push_back(std::move(submesh));
}

static void AddWeight(eng::Vertex_NorTuv_Skinned *vertex, int bone_index, float weight)
{
    std::vector<std::pair<float, uint32_t>> pairs;

    pairs.reserve(5);
    for (int i = 0; i < 4; ++i) {
        pairs.push_back(
            std::pair<float, uint32_t>(vertex->weights[i], vertex->bones[i])
        );
    }

    std::sort(
        pairs.begin(),
        pairs.end(),
        [](auto a, auto b) {
            return a.first < b.first;
        }
    );
    if (weight > pairs[0].first) {
        pairs.push_back(std::pair<float, uint32_t>(weight, bone_index));
    }
    std::sort(
        pairs.begin(),
        pairs.end(),
        [](auto a, auto b) {
            return a.first < b.first;
        }
    );
    pairs.erase(pairs.begin());

    if (pairs.size() != 4) {
        std::cerr << "std::pair vector for weights greater than 4!\n";
        return;
    }

    for (int i = 0; i < 4; ++i) {
        vertex->weights[i] = pairs[i].first;
        vertex->bones[i] = pairs[i].second;
    }
}

void RSkinnedMesh::BuildWeightsFromBone(RSkinnedSubmesh *submesh, const aiBone* ai_bone)
{
    for (int i = 0; i < ai_bone->mNumWeights; ++i) {
        const aiVertexWeight *weight = &ai_bone->mWeights[i];
        uint32_t bone_idx = BoneLookupNameIndex(ai_bone->mName.C_Str());
        if (bone_idx < 0) {
            std::cerr << "Bone \"" << ai_bone->mName.C_Str() << "\" not found\n";
            continue;
        }

        AddWeight(&submesh->vertices[weight->mVertexId], bone_idx, weight->mWeight);
    }
}

}