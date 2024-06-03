#ifndef _RENDER_BUFFER_HPP_
#define _RENDER_BUFFER_HPP_

namespace eng
{

enum class RBufferUsage
{
    kVertex,
    kIndex,
    kUniform
};

class RBuffer
{
public:
    virtual ~RBuffer() {}
    RBufferUsage GetUsage() const { return usage_; }

protected:
    RBuffer(RBufferUsage usage):
        usage_(usage)
    {}
    RBufferUsage usage_;
};

}

#endif