#include "stages/render_stage.hpp"
#include "primitives/descriptors.hpp"

#include "renderbackend.hpp"

namespace eng
{

IRenderStage::IRenderStage(
    IRenderBackend *backend_ref,
    size_t pool_size,
    RDescriptorLayoutBindingStageAccess buffers_access,
    RDescriptorLayoutBindingStageAccess textures_access
):
    backend_ref_(backend_ref),
    descriptor_set_counter_buffers_(0),
    descriptor_set_counter_textures_(0)
{
    descriptor_sets_buffers_.reserve(pool_size);
    descriptor_sets_textures_.reserve(pool_size);
    std::array<RDescriptorLayoutBinding, 4> buffer_bindings;
    std::array<RDescriptorLayoutBinding, 8> texture_bindings;
    for (int i = 0; i < buffer_bindings.size(); ++i) {
        buffer_bindings[i].bindingIndex = i;
        buffer_bindings[i].bindingStageAccessFlags = buffers_access;
        buffer_bindings[i].type = RDescriptorLayoutBindingType::kUniformBuffer;
    }

    for (int i = 0; i < texture_bindings.size(); ++i) {
        texture_bindings[i].bindingIndex = i;
        texture_bindings[i].bindingStageAccessFlags = textures_access;
        texture_bindings[i].type = RDescriptorLayoutBindingType::kTextureSampler;
    }

    descriptor_layout_buffers_ = std::unique_ptr<RDescriptorLayout>(
        backend_ref->CreateDescriptorLayout(
            buffer_bindings.data(),
            buffer_bindings.size()
        )
    );
    descriptor_layout_textures_ = std::unique_ptr<RDescriptorLayout>(
        backend_ref->CreateDescriptorLayout(
            texture_bindings.data(),
            texture_bindings.size()
        )
    );

    pools_buffers_.push_back(
        std::unique_ptr<RDescriptorPool>(
            backend_ref->CreateDescriptorPool(
                pool_size,
                RDescriptorLayoutBindingType::kUniformBuffer
            )
        )
    );
    pools_textures_.push_back(
        std::unique_ptr<RDescriptorPool>(
            backend_ref->CreateDescriptorPool(
                pool_size,
                RDescriptorLayoutBindingType::kTextureSampler
            )
        )
    );
}

static RDescriptorSet *GetSet(
    uint32_t &counter,
    std::vector<std::unique_ptr<RDescriptorSet>> &set_vector,
    RDescriptorPool *pool,
    RDescriptorLayout *layout)
{
    if (counter >= set_vector.size()) {
        set_vector.push_back(
            std::unique_ptr<RDescriptorSet>(pool->AllocateSet(layout))
        );
        return set_vector.back().get();
    }

    return set_vector[counter++].get();
}

RDescriptorSet *IRenderStage::NextSet(RDescriptorLayoutBindingType descriptor_set_type)
{
    switch(descriptor_set_type) {
    case RDescriptorLayoutBindingType::kUniformBuffer:
        return GetSet(
            descriptor_set_counter_buffers_,
            descriptor_sets_buffers_,
            pools_buffers_[0].get(),
            descriptor_layout_buffers_.get()
        );
    case RDescriptorLayoutBindingType::kTextureSampler:
        return GetSet(
            descriptor_set_counter_textures_,
            descriptor_sets_textures_,
            pools_textures_[0].get(),
            descriptor_layout_textures_.get()
        );
    }

    return nullptr;
}

void IRenderStage::ResetCounters()
{
    descriptor_set_counter_buffers_ = 0;
    descriptor_set_counter_textures_ = 0;
}

}