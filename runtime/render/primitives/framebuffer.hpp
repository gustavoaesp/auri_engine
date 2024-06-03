#ifndef _RENDER_FRAMEBUFFER_HPP_
#define _RENDER_FRAMEBUFFER_HPP_

#include "render/primitives/texture.hpp"

#include <array>
#include <memory>

namespace eng
{

static constexpr int kMaxFramebufferImages = 8;

class RFramebuffer
{
public:
    virtual ~RFramebuffer() {}

    RTexture* GetImage(int index) const {
        if (index >= num_images_) {
            return nullptr;
        }
        return images_[index].get();
    }

    RTexture* GetDepthImage() const { return depth_stencil_.get(); }

    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

protected:
    RFramebuffer(RTexture** images, int num_images, RTexture* depth):
        num_images_(num_images), depth_enabled_(depth != nullptr)
    {
        for (int i = 0; i < num_images; ++i) {
            images_[i] = std::unique_ptr<RTexture>(images[i]);
            width_ = images_[i]->GetWidth();
            height_ = images_[i]->GetHeight();
        }

        if (depth) {
            depth_stencil_ = std::unique_ptr<RTexture>(depth);
        }
    }

    int num_images_;
    bool depth_enabled_;

    std::array<std::unique_ptr<RTexture>, kMaxFramebufferImages> images_;

    std::unique_ptr<RTexture> depth_stencil_;
    uint32_t width_, height_;
};

}

#endif