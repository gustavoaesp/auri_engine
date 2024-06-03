#ifndef _RENDER_PIPELINE_HPP_
#define _RENDER_PIPELINE_HPP_

#include <array>
#include <stdint.h>

namespace eng
{

struct RDepthStencilState
{
    bool depth_test_enable;
    bool depth_write_enable;
    float min_depth;
    float max_depth;
    bool stencil_test_enable;
    bool stencil_write;
    uint32_t stencil_reference;
};

enum class RBlendFactor
{
    kZero,
    kOne,
    kSrcColor,
    kOneMinusSrcColor,
    kDstColor,
    kOneMinusDstColor,
    kSrcAlpha,
    kOneMinusSrcAlpha,
    kDstAlpha,
    kOneMinusDstAlpha,
};

enum class RBlendOp
{
    kOpAdd,
    kOpSubstract,
    kOpReverseSubstract,
    kOpMin,
    kOpMax
};

struct RBlendAttachment
{
    bool blendEnable;
    RBlendFactor srcColorBlendFactor;
    RBlendFactor dstColorBlendFactor;
    RBlendFactor srcAlphaBlendFactor;
    RBlendFactor dstAlphaBlendFactor;

    RBlendOp colorBlendOp;
    RBlendOp alphaBlendOp;
};

struct RBlendState
{
    float blend_constants[4];
    std::array<RBlendAttachment, 8> blend_attachments;
    int num_blend_attachments;
};

class RPipeline
{
public:
    virtual ~RPipeline() {}

protected:
    RPipeline() {}
};

}

#endif