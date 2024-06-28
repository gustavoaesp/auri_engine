#ifndef _RENDER_RENDER_STAGE_HPP_
#define _RENDER_RENDER_STAGE_HPP_

#include <memory>
#include <vector>

#include "primitives/descriptors.hpp"

namespace eng
{

struct RScene;

class RCommandBuffer;
class RCommandPool;
class RDescriptorLayout;
class RDescriptorPool;
class RDescriptorSet;
class RRenderPass;
class IRenderBackend;

class IRenderStage
{
public:
    virtual ~IRenderStage() {}

    virtual void Render(RScene&) = 0;

    RRenderPass *GetRenderPass() const { return render_pass_.get(); }
    RCommandBuffer *GetCommandBuffer() const { return cmd_buffer_.get(); }

protected:
    IRenderStage(
        IRenderBackend *backend_ref,
        size_t pool_size,
        RDescriptorLayoutBindingStageAccess buffers_access,
        RDescriptorLayoutBindingStageAccess textures_access
    );
    std::unique_ptr<RRenderPass> render_pass_;
    std::unique_ptr<RCommandPool> cmd_pool_;
    std::unique_ptr<RCommandBuffer> cmd_buffer_;

    RDescriptorSet *NextSet(RDescriptorLayoutBindingType);
    void ResetCounters();

    std::unique_ptr<RDescriptorLayout> descriptor_layout_buffers_;
    std::unique_ptr<RDescriptorLayout> descriptor_layout_textures_;

    IRenderBackend *backend_ref_;
private:
    std::vector<std::unique_ptr<RDescriptorPool>> pools_buffers_;
    std::vector<std::unique_ptr<RDescriptorPool>> pools_textures_;

    std::vector<std::unique_ptr<RDescriptorSet>> descriptor_sets_buffers_;
    std::vector<std::unique_ptr<RDescriptorSet>> descriptor_sets_textures_;

    uint32_t descriptor_set_counter_buffers_;
    uint32_t descriptor_set_counter_textures_;
};

}

#endif