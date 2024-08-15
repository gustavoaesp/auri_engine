#ifndef _RENDER_SKINNED_MESH_HPP_
#define _RENDER_SKINNED_MESH_HPP_

#include <math/matrix.hpp>

#include "render/mesh.hpp"

#include <stdint.h>
#include <unordered_map>
#include <vector>

class aiBone;
class aiMesh;
class aiScene;
struct BoneNode;

namespace eng
{

struct BoneNode;

struct RSkinnedSubmesh
{
    eng::RMaterial material;
    std::vector<Vertex_NorTuv_Skinned> vertices;
    std::vector<uint32_t> indices;

    std::unique_ptr<RBuffer> vertex_buffer;
    std::unique_ptr<RBuffer> index_buffer;

    std::string name;
};

class RSkinnedMesh
{
public:
    RSkinnedMesh(const aiScene *);

    struct Bone {
        int32_t parent_idx;
        std::string name;
        eng::mtx4f local_mtx;
        eng::mtx4f offset_mtx;
    };

    void PrintDebug();

    int GetSubmeshCount() const { return submeshes_.size(); }

    RSkinnedSubmesh *GetSubmesh(int idx) { return &submeshes_[idx]; }
    const RSkinnedSubmesh *GetSubmesh(int idx) const { return &submeshes_[idx]; }

    size_t GetBoneCount() const { return bones_.size(); }

    const Bone *GetBone(int idx) const {
        if (idx > bones_.size()) {
            return nullptr;
        }

        return &bones_[idx];
    }

    const Bone *GetBone(const std::string &name) const {
        int index = BoneLookupNameIndex(name);
        if (index < 0) return nullptr;
        return &bones_[index];
    }

    int GetBoneIndex(const std::string &name) const {
        return BoneLookupNameIndex(name);
    }

    const mtx4f &GetRootMatrix() const { return root_mtx_; }

private:
    std::vector<Bone> bones_;

    std::vector<RSkinnedSubmesh> submeshes_;

    void BuildBoneArray(const BoneNode *);
    int32_t BoneLookupNameIndex(const std::string &name) const;
    void BuildSubmesh(const aiMesh *);
    void BuildWeightsFromBone(RSkinnedSubmesh *, const aiBone *);

    mtx4f root_mtx_;
};

}

#endif