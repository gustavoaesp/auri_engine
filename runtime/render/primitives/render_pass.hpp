#ifndef _RENDER_RENDER_PASS_HPP_
#define _RENDER_RENDER_PASS_HPP_

namespace eng
{

class RTexture;

enum class RPassAttachmentStoreOp
{
    kStore,
    kNone,
    kDontCare
};

enum class RPassAttachmentLoadOp
{
    kLoad,
    kClear,
    kDontCare
};

struct RRenderPassAttachment
{
    RTexture *texture;
    RPassAttachmentLoadOp load_op;
    RPassAttachmentStoreOp store_op;
};

class RRenderPass
{
public:
    virtual ~RRenderPass() {}
 
protected:
    RRenderPass() {}
};

}

#endif