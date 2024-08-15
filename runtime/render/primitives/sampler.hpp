#ifndef _RENDER_PRIMITIVES_SAMPLER_HPP_
#define _RENDER_PRIMITIVES_SAMPLER_HPP_

namespace eng
{

enum class RSamplerFilterMode
{
    kFilterLinear = 0,
    kFilterNearest = 1,
    kMax32 = 0x7fffffff
};

enum class RSamplerAddressMode
{
    kRepeat = 0,
    kMirroredRepeat = 1,
    kClamp = 2,
    kMax32 = 0x7fffffff
};

struct RSamplerAttributes
{
    RSamplerFilterMode filter_mode;
    RSamplerAddressMode address_mode;
};

class RSampler
{
public:
    virtual ~RSampler() {}
};

}

#endif