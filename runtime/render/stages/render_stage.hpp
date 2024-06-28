#ifndef _RENDER_RENDER_STAGE_HPP_
#define _RENDER_RENDER_STAGE_HPP_

#include <memory>
#include <vector>

namespace eng
{

struct RScene;

class RCommandBuffer;
class RCommandPool;
class RDescriptorLayout;
class RDescriptorPool;
class RDescriptorSet;
class RRenderPass;

class IRenderStage
{
public:
    virtual ~IRenderStage() {}

    virtual void Render(RScene&) = 0;

    RRenderPass *GetRenderPass() const { return render_pass_.get(); }
    RCommandBuffer *GetCommandBuffer() const { return cmd_buffer_.get(); }

protected:
    IRenderStage() {}
    std::unique_ptr<RRenderPass> render_pass_;
    std::unique_ptr<RCommandPool> cmd_pool_;
    std::unique_ptr<RCommandBuffer> cmd_buffer_;
};

}

#endif