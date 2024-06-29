#include "core/global_context.hpp"

#include "render/mesh_file.hpp"
#include "render/mesh.hpp"
#include "render/renderer.hpp"

#include <fstream>
#include <string>
#include <string.h>
#include <vector>

namespace eng
{

RMesh *RFileMeshRead(IRenderBackend *backend, const char *filename)
{
    std::ifstream infile(filename, std::ios::in);
    eng::RFileMesh file_header;
    std::vector<eng::RFileSubmesh> submeshes;

    infile.read((char*)&file_header, sizeof(RFileMesh));
    if (memcmp(file_header.signature, kMeshFileSignature, 4)) {
        return nullptr;
    }

    submeshes = std::vector<eng::RFileSubmesh>(file_header.num_submeshes);
    infile.read((char*)submeshes.data(), sizeof(eng::RFileSubmesh) * submeshes.size());

    RMesh *mesh = new RMesh();
    for (const auto& submesh: submeshes) {
        std::vector<Vertex_NorTuv> vertices(submesh.num_vertices);
        std::vector<uint32_t> indices(submesh.num_indices);
        RMaterial material;
        material.diffuse = g_context->texture_manager->Get(submesh.material.diffuse_tex);

        infile.read((char*)vertices.data(), submesh.num_vertices * sizeof(Vertex_NorTuv));
        infile.read((char*)indices.data(), submesh.num_indices * sizeof(uint32_t));

        mesh->AddSubmesh(std::make_unique<RSubmesh>(
            backend, RVertexType::kVertexPos3Nor3Tex2,
            vertices.data(), vertices.size(),
            indices.data(), indices.size(),
            std::move(material)
        ));
    }

    return mesh;
}

}