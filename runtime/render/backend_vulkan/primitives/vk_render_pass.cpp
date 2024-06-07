#include "backend_vulkan/primitives/vk_render_pass.hpp"
#include "backend_vulkan/vk_swapchain.hpp"
#include "backend_vulkan/format_lookup.hpp"
#include "backend_vulkan/primitives/vk_texture.hpp"

#include <array>

namespace eng
{

VulkanRenderPass::VulkanRenderPass(VkDevice vk_device,
    const VulkanSwapchain* swapchain,
    bool depth_test
):
    vk_device_ref(vk_device)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain->getFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = (depth_test) ? &depth_attachment_ref : nullptr;

    VkAttachmentDescription attachments[] = {
        colorAttachment,
        depth_attachment
    };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1 + depth_test;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vk_device, &renderPassInfo, nullptr, &vk_render_pass)
        != VK_SUCCESS)
    {
        //ExitError("Can't create render pass");
        // TODO error
    }
}

VulkanRenderPass::VulkanRenderPass(
    VkDevice vk_device,
    const RRenderPassAttachment *color_attachments,
    int num_attachments,
    const RRenderPassAttachment *depth_attachment):
    vk_device_ref(vk_device)
{
    std::vector<VkAttachmentDescription> vk_attachments;
    std::vector<VkAttachmentReference> vk_attachment_refs;
    vk_attachments.reserve(num_attachments + (depth_attachment != nullptr));
    vk_attachment_refs.reserve(num_attachments + (depth_attachment != nullptr));

    for(int i = 0; i < num_attachments; ++i) {
        VulkanTexture *attachment = static_cast<VulkanTexture*>(color_attachments[i].texture);
        vk_attachments.push_back({});
        vk_attachment_refs.push_back({});
        VkAttachmentDescription& desc = vk_attachments.back();
        desc.format = attachment->vk_fmt;

        desc.samples = VK_SAMPLE_COUNT_1_BIT;

        switch(color_attachments[i].load_op) {
        case RPassAttachmentLoadOp::kClear:
            desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        case RPassAttachmentLoadOp::kLoad:
            desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case RPassAttachmentLoadOp::kDontCare:
            desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        }
        switch(color_attachments[i].store_op) {
        case RPassAttachmentStoreOp::kNone:
            desc.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
            break;
        case RPassAttachmentStoreOp::kStore:
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case RPassAttachmentStoreOp::kDontCare:
            desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        }

        desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference& attRef = vk_attachment_refs.back();
        attRef.attachment = i; 
        attRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if(depth_attachment != nullptr) {
        vk_attachments.push_back({});
        VkAttachmentDescription& vk_depth_attachment = vk_attachments.back();

        vk_depth_attachment.format = (static_cast<VulkanTexture*>(depth_attachment->texture))->vk_fmt;
        vk_depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        vk_depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        vk_depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vk_depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vk_depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vk_depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk_depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = num_attachments;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = vk_attachment_refs.size();
    subpass.pColorAttachments = vk_attachment_refs.data();
    subpass.pDepthStencilAttachment = (depth_attachment != nullptr) ? &depth_attachment_ref : nullptr;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = vk_attachments.size();
    renderPassInfo.pAttachments = vk_attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;

    dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(vk_device, &renderPassInfo, nullptr, &vk_render_pass)
        != VK_SUCCESS)
    {
        // TODO error
    }
}

VulkanRenderPass::~VulkanRenderPass()
{
    vkDestroyRenderPass(vk_device_ref, vk_render_pass, nullptr);
}

}
