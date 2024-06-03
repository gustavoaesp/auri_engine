#ifndef _RENDER_TEXTURE_HPP_
#define _RENDER_TEXTURE_HPP_

#include <stdint.h>

#include "render/format.hpp"

namespace eng
{

class RTexture
{
public:
    virtual ~RTexture() {}

    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

    EFormat GetFormat() const { return fmt_; }
protected:
    RTexture(uint32_t width, uint32_t height, EFormat format):
        width_(width), height_(height), fmt_(format)
    {}
    uint32_t width_, height_;
    EFormat fmt_;
};

}

#endif