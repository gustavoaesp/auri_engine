#ifndef _RENDER_SHADER_HPP_
#define _RENDER_SHADER_HPP_

#include <stdint.h>

namespace eng
{

enum class RShaderPipelineBind
{
    kShaderVertex,
    kShaderFragment
};

class RShader
{
public:
    virtual ~RShader() {}

    RShaderPipelineBind GetPipelineBind() const { return pipeline_bind_; }

protected:
    RShader(RShaderPipelineBind pipeline_bind):
        pipeline_bind_(pipeline_bind)
    {}

    RShaderPipelineBind pipeline_bind_;
};

}

#endif