#include <filesystem>
#include <fstream>
#include <iostream>
#include <ofbx/ofbx.h>
#include <memory>
#include <vector>
#include <string.h>

#include "render/mesh_file.hpp"

std::vector<uint8_t> contents;

void print_spaces(int level)
{
    for (int i = 0; i < level; ++i)
        std::cout << "    ";
}

ofbx::IScene* loadScene(const std::string& filename)
{
    std::ifstream file(filename,  std::ios::binary | std::ios::ate);
    size_t filesize = 0;

    if (!file) {
        std::cerr << "Error opening \"" << filename << "\"\n";
        return nullptr;
    }

    filesize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (!filesize) {
        std::cerr << "Empty file!\n";
        return nullptr;
    }
    contents = std::vector<uint8_t>(filesize);
    file.read((char*)contents.data(), contents.size());

    ofbx::LoadFlags flags =
		ofbx::LoadFlags::TRIANGULATE |
//		ofbx::LoadFlags::IGNORE_MODELS |
		ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
		ofbx::LoadFlags::IGNORE_CAMERAS |
		//ofbx::LoadFlags::IGNORE_LIGHTS |
//		ofbx::LoadFlags::IGNORE_TEXTURES |
		ofbx::LoadFlags::IGNORE_SKIN |
		ofbx::LoadFlags::IGNORE_BONES |
		ofbx::LoadFlags::IGNORE_PIVOTS |
//		ofbx::LoadFlags::IGNORE_MATERIALS |
		ofbx::LoadFlags::IGNORE_POSES |
		ofbx::LoadFlags::IGNORE_VIDEOS |
		ofbx::LoadFlags::IGNORE_LIMBS |
//		ofbx::LoadFlags::IGNORE_MESHES |
		ofbx::LoadFlags::IGNORE_ANIMATIONS;

    ofbx::IScene* scene = ofbx::load(contents.data(), contents.size(), (ofbx::u16) flags);

    return scene;
}

void IterateTexture(const ofbx::Object& obj, int level)
{
    const ofbx::Texture& texture = (const ofbx::Texture&) obj;
    char filename[128];
    print_spaces(level + 1);
    texture.getFileName().toString(filename);
    std::cout << "filename: \"" << filename << "\"\n";
}

void IterateMaterial(const ofbx::Material& mat, int level)
{
    const ofbx::Texture* diff = mat.getTexture(ofbx::Texture::TextureType::DIFFUSE) ;
    IterateTexture(*diff, level + 1);
}

void IterateMesh(const ofbx::Object& obj, int level)
{
    const ofbx::Mesh& mesh = (const ofbx::Mesh&) obj;

    print_spaces(level + 1);
    std::cout << "Name: " << mesh.name << "\n";

    //IterateMaterial(*mesh.getMaterial(0), level + 1);
}

void IterateGeometry(const ofbx::Object& obj, int level)
{
    const ofbx::Geometry& geometry = (const ofbx::Geometry&) obj;

    int indexCount = geometry.getIndexCount();
    int vertexCount = geometry.getVertexCount();

    print_spaces(level + 1);
    std::cout << "Index count: " << indexCount << "  VertexCount: " << vertexCount << "\n";
    print_spaces(level + 1);
    std::cout << "indices:\n";
    const int* indices = geometry.getFaceIndices();
    const ofbx::Vec3* vertices = geometry.getVertices();

/*
    for (int i = 0; i < indexCount; ++i) {
        int index = indices[i];
        if (index < 0) {
            index = ~index;
        }
        const ofbx::Vec3& vec = vertices[index];
        print_spaces(level + 1);
        printf("{%.4f, %.4f, %.4f} (%d)\n", vec.x, vec.y, vec.z, index);
    }*/

}

void ProcessMesh(const ofbx::Mesh& mesh)
{
    if (!mesh.getMaterialCount()) {
        std::cerr << "Error: no material in mesh\n";
    }

    const auto* geometry = mesh.getGeometry();

/*
    eng::FileMesh file_mesh;
    file_mesh.vtx_type = eng::E_VTX_POS3_NOR3_TEX2;
    file_mesh.childCount = 0;
    file_mesh.indexCount = geometry->getIndexCount();
    file_mesh.vertexCount = geometry->getVertexCount();

    const ofbx::Material* material = mesh.getMaterial(0);
    const ofbx::Texture* diffuse = material->getTexture(ofbx::Texture::TextureType::DIFFUSE);
    char diffuse_filename[1024] = {};
    diffuse->getFileName().toString(diffuse_filename);
    // TODO change to the internal format extension
    file_mesh.material.spec_exp = 0;

    std::filesystem::path p = diffuse_filename;
    auto diffuse_file = p.filename();
    diffuse_file.replace_extension("tex");
    diffuse_file = "textures" / diffuse_file;

    strcpy(file_mesh.material.diffuse_texture, diffuse_file.c_str());

    std::vector<eng::Vertex_NorTuv> vertices(file_mesh.vertexCount);
    std::vector<int> indices(file_mesh.indexCount);

    const ofbx::Vec3* fbx_vertices = geometry->getVertices();
    const int* fbx_indices = geometry->getFaceIndices();

    // Stupid ass conversion
    for (int i = 0; i < file_mesh.vertexCount; ++i) {
        vertices[i].position(0) = fbx_vertices[i].x;
        vertices[i].position(1) = fbx_vertices[i].z;
        vertices[i].position(2) = -fbx_vertices[i].y;
    }

    for (int i = 0; i < file_mesh.indexCount; ++i) {
        int index = fbx_indices[i];
        if (index < 0) {
            index = ~index;
        }
        indices[i] = index;

        vertices[index].normal(0) = geometry->getNormals()[i].x;
        vertices[index].normal(1) = geometry->getNormals()[i].z;
        vertices[index].normal(2) = -geometry->getNormals()[i].y;

        vertices[index].texel(0) = geometry->getUVs()[i].x;
        vertices[index].texel(1) = geometry->getUVs()[i].y;
    }

    eng::FileRoot root;
    root.signature = eng::kSignature;
    root.top_level_meshes = 1;*/

    int total_vertex_count = geometry->getVertexCount();
    int total_index_count = geometry->getIndexCount();

    int num_materials = mesh.getMaterialCount();
    fprintf(stderr, "num_materials: %d\n", num_materials);
    std::vector<std::vector<eng::Vertex_NorTuv>> submesh_geometry(num_materials);
    std::vector<eng::FileSubmesh> submesh_structs(num_materials);

    const ofbx::Vec3* vertices = geometry->getVertices();
    const ofbx::Vec3* normals = geometry->getNormals();
    const ofbx::Vec2* uvs = geometry->getUVs();
    const int* fbx_indices = geometry->getFaceIndices();

    for(int i = 0; i < total_vertex_count; ++i) {
        int index = fbx_indices[i];
        if (index < 0) {
            index = ~index;
        }

        int face_index = i / 3;
        int material_index = (geometry->getMaterials())?
            geometry->getMaterials()[face_index] : 0;
        submesh_geometry[material_index].push_back(
            eng::Vertex_NorTuv{
                .position = eng::vec3f(vertices[index].x, vertices[index].z, -vertices[index].y),
                .normal = eng::vec3f(normals[i].x, normals[i].z, -normals[i].y),
                .texel = eng::vec2f(uvs[i].x, 1.0f - uvs[i].y),
            }
        );
    }

    for (int i = 0; i < num_materials; ++i) {
        const ofbx::Material* material = mesh.getMaterial(i);

        submesh_structs[i].vertexCount = submesh_geometry[i].size();
        submesh_structs[i].material.active = 1;
        submesh_structs[i].material.ambient = 0xffffffff;
        submesh_structs[i].vtx_type = eng::E_VTX_POS3_NOR3_TEX2;

        const ofbx::Texture* diffuse =
            material->getTexture(ofbx::Texture::TextureType::DIFFUSE);
        if(diffuse) {
            char diffuse_filename[512] = {};
            diffuse->getFileName().toString(diffuse_filename);
            // change to the internal format extension
            submesh_structs[i].material.spec_exp = 0;
            std::filesystem::path p = diffuse_filename;
            auto diffuse_file = p.filename();
            diffuse_file.replace_extension("tex");
            diffuse_file = "textures" / diffuse_file;
            strcpy(submesh_structs[i].material.diffuse_texture, diffuse_file.c_str());
        } else {
            submesh_structs[i].material.diffuse_texture[0] = 0;
        }
    }

    eng::FileRoot root;
    root.signature = eng::kSignature;
    root.top_level_meshes = 1;

    eng::FileMesh file_mesh;
    file_mesh.childCount = 1;
    file_mesh.submesh_count = num_materials;

    {
        std::ofstream file("out.c3d", std::ios::binary);

        file.write((char*)&root, sizeof(eng::FileRoot));
        file.write((char*)&file_mesh, sizeof(eng::FileMesh));
        /*
        file.write((char*)vertices.data(), sizeof(eng::Vertex_NorTuv) * file_mesh.vertexCount);
        file.write((char*)indices.data(), sizeof(int) * file_mesh.indexCount);*/
        for (int i = 0; i < num_materials; ++i) {
            file.write((char*)&submesh_structs[i], sizeof(eng::FileSubmesh));
            file.write((char*)submesh_geometry[i].data(),
                sizeof(eng::Vertex_NorTuv) * submesh_geometry[i].size()
            );
        }
    }
}

void IterateObj(const ofbx::Object& obj, int level)
{
    const char* label;
    switch(obj.getType())
    {
        case ofbx::Object::Type::GEOMETRY:
            //IterateGeometry((const ofbx::Geometry&) obj);
            label = "geometry"; break;
        case ofbx::Object::Type::MESH: label = "mesh"; break;
        case ofbx::Object::Type::MATERIAL:
            //IterateMaterial((const ofbx::Material&) obj);
            label = "material"; break;
        case ofbx::Object::Type::ROOT: label = "root"; break;
        case ofbx::Object::Type::TEXTURE: label = "texture"; break;
        case ofbx::Object::Type::NULL_NODE: label = "null"; break;
        case ofbx::Object::Type::LIMB_NODE: label = "limb node"; break;
        case ofbx::Object::Type::NODE_ATTRIBUTE: label = "node attribute"; break;
        case ofbx::Object::Type::CLUSTER: label = "cluster"; break;
        case ofbx::Object::Type::SKIN: label = "skin"; break;
        case ofbx::Object::Type::ANIMATION_STACK: label = "animation stack"; break;
        case ofbx::Object::Type::ANIMATION_LAYER: label = "animation layer"; break;
        case ofbx::Object::Type::ANIMATION_CURVE: label = "animation curve"; break;
        case ofbx::Object::Type::ANIMATION_CURVE_NODE: label = "animation curve node"; break;
        case ofbx::Object::Type::LIGHT: label = "light"; break;
        case ofbx::Object::Type::CAMERA: label = "camera"; break;
        default: exit(-1); break;
    }

    print_spaces(level);
    std::cout << "* Found a " << label << "\n";

    if (obj.getType() == ofbx::Object::Type::MESH) {
        IterateMesh(obj, level);
        ProcessMesh((const ofbx::Mesh&)obj);
    }
    else if (obj.getType() == ofbx::Object::Type::GEOMETRY) {
        IterateGeometry(obj, level);
    }
    else if (obj.getType() == ofbx::Object::Type::TEXTURE) {
        IterateTexture(obj, level);
    }

    int i = 0;
    while(ofbx::Object* child = obj.resolveObjectLink(i)) {
        IterateObj(*child, level + 1);
        ++i;
    }
}

void IterateScene(const ofbx::IScene& scene)
{
    const ofbx::Object* root = scene.getRoot();
    if (root) {
        IterateObj(*root, 0);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Not enough args!\n";
        return -1;
    }

    ofbx::IScene* scene = loadScene(argv[1]);
    IterateScene(*scene);

    return 0;
}
