#include "mesh.hpp"

namespace eng
{

RSubmesh::RSubmesh(IRenderBackend *backend_ref, RVertexType vertex_type,
    void *vertex_contents, uint32_t vertex_count,
    void *index_contents, uint32_t index_count, RMaterial &&mat
) : material(std::move(mat)),
    vertices_count(vertex_count),
    indices_count(index_count),
    vertex_type(vertex_type)
{
    size_t vertex_size = GetVertexSize(vertex_type);
    vertex_buffer = std::unique_ptr<RBuffer>(
        backend_ref->CreateBuffer(
            RBufferUsage::kVertex,
            vertex_size * vertex_count,
            vertex_contents
        )
    );
    index_buffer = std::unique_ptr<RBuffer>(
        backend_ref->CreateBuffer(
            RBufferUsage::kIndex,
            index_count * sizeof(uint32_t),
            index_contents
        )
    );
}

RSubmesh *RMesh::GetSubmesh(uint32_t index)
{
    return submeshes_[index].get();
}

void RMesh::AddSubmesh(std::unique_ptr<RSubmesh> &&submesh)
{
    submeshes_.push_back(std::move(submesh));
}

}