#ifndef _RENDER_MESH_HPP_
#define _RENDER_MESH_HPP_

#include <memory>
#include <vector>

#include "primitives/buffer.hpp"
#include "primitives/texture.hpp"
#include "primitives/pipeline.hpp"
#include "renderbackend.hpp"

namespace eng
{

struct RMaterial
{
    std::shared_ptr<RTexture> diffuse;
    std::string diffuse_filename;
    std::shared_ptr<RPipeline> pipeline; //can be null
};

struct RSubmesh
{
    RSubmesh(
        IRenderBackend *, RVertexType vertex_type,
        void *vertex_contents, uint32_t vertex_count,
        void *index_contents, uint32_t index_count,
        RMaterial &&mat
    );

    std::unique_ptr<RBuffer> vertex_buffer;
    std::unique_ptr<RBuffer> index_buffer;

    uint32_t indices_count;
    uint32_t vertices_count;
    RVertexType vertex_type;

    RMaterial material;
};

class RMesh
{
public:
    uint32_t GetSubmeshCount() const { return submeshes_.size(); };
    RSubmesh *GetSubmesh(uint32_t index);

    void AddSubmesh(std::unique_ptr<RSubmesh>&&);
private:
    std::vector<std::unique_ptr<RSubmesh>> submeshes_;

    friend class RRenderer;
};

}

#endif