#ifndef _RENDER_MESH_FILE_HPP_
#define _RENDER_MESH_FILE_HPP_

#include <stdint.h>

#include "render/renderbackend.hpp"

namespace eng
{

static const char *kMeshFileSignature = "AURI";

struct RFileMesh
{
    char signature[4];
    uint32_t num_submeshes;
};

struct RFileSubmeshMaterial
{
    char diffuse_tex[256];
};

struct RFileSubmesh
{
    RFileSubmeshMaterial material;
    uint32_t num_vertices;
    uint32_t num_indices;
};

class RMesh *RFileMeshRead(IRenderBackend*, const char *filename);

}

#endif