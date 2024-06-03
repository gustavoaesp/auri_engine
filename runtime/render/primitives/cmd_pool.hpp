#ifndef _RENDER_CMD_POOL_HPP_
#define _RENDER_CMD_POOL_HPP_

#include <memory>

namespace eng
{

class RCommandBuffer;

class RCommandPool
{
public:
    virtual ~RCommandPool() {}

    virtual RCommandBuffer *CreateCommandBuffer(bool primary) = 0;

protected:
    RCommandPool() {}
};

}

#endif