#include <filesystem>
#include <fstream>
#include <iostream>

#include "render/texture_file.hpp"
/* Needed */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fs = std::filesystem;

void perform_op(const char* filename)
{
    fs::path file = filename;
    fs::path output = file.parent_path();
    std::string new_filename = file.stem().string() + ".tex";
    output /= new_filename;

    eng::RTextureFileHeader texFile;
    uint8_t* pixels = stbi_load
    (
        file.string().c_str(),
        &texFile.width,
        &texFile.height,
        &texFile.channels,
        STBI_rgb_alpha
    );

    fprintf(stderr, "channels: %d\n", texFile.channels);

    std::ofstream outputFile(output, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Could not open/create " << output << " for writing\n";
        return;
    }
    outputFile.write((char*)&texFile, sizeof(eng::RTextureFileHeader));
    outputFile.write((char*)pixels, texFile.width * texFile.height * 4);

    std::cout << "Converted " << file << " into " << output << "\n";
    stbi_image_free(pixels);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "No images provided\n";
        return -1;
    }

    for (int i = 1; i < argc; ++i) {
        perform_op(argv[i]);
    }

    return 0;
}
