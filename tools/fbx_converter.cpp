#include <filesystem>
#include <fstream>
#include <iostream>
#include <ofbx/ofbx.h>
#include <memory>
#include <vector>
#include <string.h>

#include <core/game_mode.hpp>
#include <render/mesh_file.hpp>
#include <render/vertex.hpp>

namespace eng
{
    std::unique_ptr<CGameMode> g_game_mode;
}

struct Material
{
    std::string diffuse;
};

struct Submesh
{
    Material mat;
    std::vector<eng::Vertex_NorTuv> vertices;
    std::vector<uint32_t> indices;
};

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
//		ofbx::LoadFlags::IGNORE_BONES |
		ofbx::LoadFlags::IGNORE_PIVOTS |
//		ofbx::LoadFlags::IGNORE_MATERIALS |
		ofbx::LoadFlags::IGNORE_POSES |
		ofbx::LoadFlags::IGNORE_VIDEOS;
//		ofbx::LoadFlags::IGNORE_LIMBS |
//		ofbx::LoadFlags::IGNORE_MESHES |
//		ofbx::LoadFlags::IGNORE_ANIMATIONS;

    ofbx::IScene* scene = ofbx::load(contents.data(), contents.size(), (ofbx::u16) flags);

    return scene;
}


void IterateLimb(const ofbx::Object &obj, int level)
{
    print_spaces(level + 1);
    std::cout << "* Name :" << obj.name << "\n";
}

void IterateAnimationCurve(const ofbx::Object &obj, int level)
{
    const ofbx::AnimationCurve &anim = (const ofbx::AnimationCurve&)obj;
    print_spaces(level + 1);
    std::cout << "* key_count: " << anim.getKeyCount() << "\n";
}

void IterateAnimationCurveNode(const ofbx::Object &obj, int level)
{
    const ofbx::AnimationCurveNode &anim_node = (const ofbx::AnimationCurveNode &)obj;

    print_spaces(level + 1);
    std::cout << "* Name: " << obj.name <<"\n";
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

    IterateMaterial(*mesh.getMaterial(0), level + 1);
}

void IterateGeometry(const ofbx::Object& obj, int level)
{
    const ofbx::Geometry& geometry = (const ofbx::Geometry&) obj;

    int indexCount = geometry.getIndexCount();
    int vertexCount = geometry.getVertexCount();

    print_spaces(level + 1);
    std::cout << "Index count: " << indexCount << "  VertexCount: " << vertexCount << "\n";
}

std::string ConvertTextureFilename(const char *filename)
{
    std::filesystem::path p = filename;
    auto diffuse_file = p.filename();
    diffuse_file.replace_extension("tex");
    diffuse_file = "textures" / diffuse_file;

    return diffuse_file.string();
}

static std::vector<std::unique_ptr<Submesh>> submeshes;

std::unique_ptr<Submesh> ProcessSubmesh(const ofbx::Mesh& mesh)
{
    const auto* geometry = mesh.getGeometry();
    auto submesh = std::make_unique<Submesh>();
    if (!mesh.getMaterialCount()) {
        std::cerr << "Error: no material in mesh\n";
        return nullptr;
    }
    const ofbx::Material *material = mesh.getMaterial(0);
    const ofbx::Texture *diffuse = material->getTexture(ofbx::Texture::TextureType::DIFFUSE);

    submesh->indices = std::vector<uint32_t>(geometry->getIndexCount());
    submesh->vertices = std::vector<eng::Vertex_NorTuv>(geometry->getVertexCount());

    char diffuse_filename[1024];
    diffuse->getFileName().toString(diffuse_filename);

    submesh->mat.diffuse = ConvertTextureFilename(diffuse_filename);


    const ofbx::Vec3* fbx_vertices = geometry->getVertices();
    const int* fbx_indices = geometry->getFaceIndices();

    // Stupid ass conversion
    for (int i = 0; i < submesh->vertices.size(); ++i) {
        submesh->vertices[i].position(0) = fbx_vertices[i].x;
        submesh->vertices[i].position(1) = fbx_vertices[i].z;
        submesh->vertices[i].position(2) = -fbx_vertices[i].y;
    }

    for (int i = 0; i < submesh->indices.size(); ++i) {
        int index = fbx_indices[i];
        if (index < 0) {
            index = ~index;
        }
        submesh->indices[i] = index;

        submesh->vertices[index].normal(0) = geometry->getNormals()[i].x;
        submesh->vertices[index].normal(1) = geometry->getNormals()[i].z;
        submesh->vertices[index].normal(2) = -geometry->getNormals()[i].y;

        submesh->vertices[index].texel(0) = geometry->getUVs()[i].x;
        submesh->vertices[index].texel(1) = (1.0f - geometry->getUVs()[i].y);
    }

    return std::move(submesh);
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
        auto submesh = ProcessSubmesh((const ofbx::Mesh&)obj);
        if (submesh) {
            submeshes.push_back(std::move(submesh));
        }
    }
    else if (obj.getType() == ofbx::Object::Type::GEOMETRY) {
        IterateGeometry(obj, level);
    }
    else if (obj.getType() == ofbx::Object::Type::TEXTURE) {
        IterateTexture(obj, level);
    } else if (obj.getType() == ofbx::Object::Type::LIMB_NODE) {
        IterateLimb(obj, level);
    } else if (obj.getType() == ofbx::Object::Type::ANIMATION_CURVE) {
        IterateAnimationCurve(obj, level);
    } else if (obj.getType() == ofbx::Object::Type::ANIMATION_CURVE_NODE) {
        IterateAnimationCurveNode(obj, level);
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

void SerializeSubmeshes(const char *outfilename)
{
    std::ofstream file_out(outfilename, std::ios::binary);
    if (!file_out) {
        return;
    }

    eng::RFileMesh file_header;
    memcpy(file_header.signature, eng::kMeshFileSignature, 4);
    file_header.num_submeshes = submeshes.size();

    file_out.write((char*)&file_header, sizeof(eng::RFileMesh));

    for (const auto &submesh : submeshes) {
        eng::RFileSubmesh submesh_serialized{};
        strcpy(submesh_serialized.material.diffuse_tex, submesh->mat.diffuse.c_str());
        submesh_serialized.num_vertices = submesh->vertices.size();
        submesh_serialized.num_indices = submesh->indices.size();
        file_out.write((char*)&submesh_serialized, sizeof(eng::RFileSubmesh));
    }

    for (const auto &submesh : submeshes) {
        file_out.write((char*)submesh->vertices.data(), submesh->vertices.size() * sizeof(eng::Vertex_NorTuv));
        file_out.write((char*)submesh->indices.data(), submesh->indices.size() * sizeof(uint32_t));
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

    //SerializeSubmeshes("out.c3d");

    return 0;
}
