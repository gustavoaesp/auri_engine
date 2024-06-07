#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include <stdint.h>

namespace eng
{

struct RTextureFileHeader
{
    int32_t width, height;
    int32_t channels;
};

struct RTextureFile
{
    RTextureFileHeader header;
    std::vector<uint8_t> data;
};

static RTextureFile RTextureRead(const char* filename)
{
    RTextureFile texture;
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Texture not found: \"" << filename << "\n";
        texture.header.width = 4;
        texture.header.height = 4;
        texture.header.width = 4;
        texture.data = {
            0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
            0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
            0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
            0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd
        };
        return texture;
    }

    file.read((char*)&texture.header, sizeof(RTextureFileHeader));
    size_t pixelsBytes = texture.header.width * texture.header.height * 4;
    texture.data = std::vector<uint8_t>(pixelsBytes);
    file.read((char*)texture.data.data(), pixelsBytes);

    return texture;
}

}
