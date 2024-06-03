#ifndef _RENDER_PRIMITIVES_SAMPLER_HPP_
#define _RENDER_PRIMITIVES_SAMPLER_HPP_

namespace eng
{

enum class RSamplerFilterMode
{
    kFilterLinear,
    kFilterNearest
};

enum class RSamplerAddressMode
{
    kRepeat,
    kMirroredRepeat,
    kClamp
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