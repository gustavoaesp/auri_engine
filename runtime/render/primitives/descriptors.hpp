#ifndef _RENDER_DESCRIPTORS_HPP_
#define _RENDER_DESCRIPTORS_HPP_

#include <array>

#include <stdint.h>

namespace eng
{

static constexpr uint32_t kMaxDescriptorBindings = 8;

class RBuffer;
class RTexture;
class RSampler;

enum class RDescriptorLayoutBindingType
{
    kUniformBuffer,
    kTextureSampler
};

enum RDescriptorLayoutBindingStageAccess
{
    EShaderStageVertexBit       = 0x001,
    EShaderStageFragmentBit     = 0x002
};

struct RDescriptorLayoutBinding
{
    RDescriptorLayoutBindingType type;
    uint32_t bindingIndex;
    uint32_t bindingStageAccessFlags;
};

struct RTextureSamplerBinding
{
    RTexture* texture;
    RSampler* sampler;
};

struct RBufferBinding
{
    RBuffer* buffer;
    uint32_t start_offset;
    uint32_t size;
};

class RDescriptorLayout
{
public:
    virtual ~RDescriptorLayout() {}

protected:
    RDescriptorLayout() {}
};

class RDescriptorPool
{
public:
    virtual ~RDescriptorPool() {}

protected:
    RDescriptorPool() {}
};

class RDescriptorSet
{
public:
    virtual ~RDescriptorSet() {}

    virtual void BindBuffers(
        uint32_t start_index,
        const RBufferBinding*,
        uint32_t count
    ) = 0;

    virtual void BindTextures(
        uint32_t start_index,
        const RTextureSamplerBinding*,
        uint32_t count
    ) = 0;

protected:
    RDescriptorSet() {}
};

}

#endif