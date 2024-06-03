#include "backend_vulkan/primitives/vk_pipeline.hpp"

namespace eng
{

VkBlendFactor _VulkanConvertBlendFactor(RBlendFactor blend_factor)
{
    switch(blend_factor) {
    case RBlendFactor::kDstAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case RBlendFactor::kDstColor:
        return VK_BLEND_FACTOR_DST_COLOR;
    case RBlendFactor::kOne:
        return VK_BLEND_FACTOR_ONE;
    case RBlendFactor::kOneMinusDstAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case RBlendFactor::kOneMinusDstColor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case RBlendFactor::kOneMinusSrcAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case RBlendFactor::kOneMinusSrcColor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case RBlendFactor::kSrcAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case RBlendFactor::kSrcColor:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case RBlendFactor::kZero:
        return VK_BLEND_FACTOR_ZERO;
    }

    return VK_BLEND_FACTOR_ONE;
}

VkBlendOp _VulkanConvertBlendOp(RBlendOp blend_op)
{
    switch(blend_op) {
    case RBlendOp::kOpAdd:
        return VK_BLEND_OP_ADD;
    case RBlendOp::kOpSubstract:
        return VK_BLEND_OP_SUBTRACT;
    case RBlendOp::kOpReverseSubstract:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case RBlendOp::kOpMin:
        return VK_BLEND_OP_MIN;
    case RBlendOp::kOpMax:
        return VK_BLEND_OP_MAX;
    }

    return VK_BLEND_OP_ADD;
}

VulkanPipeline::VulkanPipeline(
    VkDevice vk_device,
    const VulkanRenderPass *render_pass, const RBlendState *blend_state,
    const RDepthStencilState *depth_state,
    const VulkanShader **shaders, uint32_t num_shaders,
    const VulkanVertexDescription *input_description,
    const VulkanDescriptorSetLayout **descriptor_set_layouts,
    int num_descriptor_set_layouts
):
    vk_device_ref(vk_device)
{
    std::array<VkPipelineShaderStageCreateInfo, 8> vk_shader_stages{};
    std::array<VkDynamicState, 2> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo vk_dynamic_state{};
    VkPipelineVertexInputStateCreateInfo vk_input_state{};
    VkPipelineInputAssemblyStateCreateInfo vk_input_assembly{};
    VkViewport vk_viewport{};
    VkRect2D vk_scissor{};
    VkPipelineViewportStateCreateInfo vk_viewport_state{};
    VkPipelineRasterizationStateCreateInfo vk_rasterizer{};
    VkPipelineMultisampleStateCreateInfo vk_multisampling{};
    VkPipelineDepthStencilStateCreateInfo vk_depth_stencil{};
    std::array<VkPipelineColorBlendAttachmentState, 8> vk_blend_color_attachments;
    VkPipelineColorBlendStateCreateInfo vk_color_blending{};
    VkPushConstantRange vk_push_constant_range{};
    VkPipelineLayoutCreateInfo vk_pipeline_layout_info{};
    std::array<VkDescriptorSetLayout, 4> vk_set_layouts{ };
    VkGraphicsPipelineCreateInfo vk_pipeline_info{};

    for (int i = 0; i < num_shaders; ++i) {
        vk_shader_stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vk_shader_stages[i].module = shaders[i]->vk_shader_module;

        switch(shaders[i]->GetPipelineBind()) {
        case RShaderPipelineBind::kShaderVertex:
            vk_shader_stages[i].pName = "main";
            vk_shader_stages[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case RShaderPipelineBind::kShaderFragment:
            vk_shader_stages[i].pName = "main";
            vk_shader_stages[i].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        }
    }

    // DYNAMIC STATES
    vk_dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    vk_dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    vk_dynamic_state.pDynamicStates = dynamic_states.data();

    // VERTEX DESCRIPTORS
    vk_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (input_description) {
        vk_input_state.pVertexBindingDescriptions =
            input_description->vertex_binding_descriptors.data();
        vk_input_state.vertexBindingDescriptionCount =
            input_description->vertex_binding_count;

        vk_input_state.pVertexAttributeDescriptions =
            input_description->vertex_attributes_descriptors.data();
        vk_input_state.vertexAttributeDescriptionCount =
            input_description->vertex_attribute_count;
    }

    // INPUT ASSEMBLY
    vk_input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vk_input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vk_input_assembly.primitiveRestartEnable = VK_FALSE;

    // VIEWPORT
    vk_viewport.x = 0;
    vk_viewport.y = 0;
    vk_viewport.width = 1;
    vk_viewport.height = 1;
    vk_viewport.minDepth = 0.0f;
    vk_viewport.maxDepth = 1.0f;

    // SCISSOR
    vk_scissor.offset = {0, 0};
    vk_scissor.extent = {0, 0};

    // Viewport + Scissor stage
    vk_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vk_viewport_state.viewportCount = 1;
    vk_viewport_state.pViewports = &vk_viewport;
    vk_viewport_state.scissorCount = 1;
    vk_viewport_state.pScissors = &vk_scissor;

    // Rasterizer
    vk_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vk_rasterizer.depthClampEnable = VK_FALSE;
    vk_rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    vk_rasterizer.lineWidth = 1.0f;
    vk_rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    vk_rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    vk_rasterizer.depthBiasEnable = VK_FALSE;
    vk_rasterizer.depthBiasConstantFactor = 0.0f;
    vk_rasterizer.depthBiasClamp = 0.0f;
    vk_rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling
    // TODO allow multisampling
    vk_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vk_multisampling.sampleShadingEnable = VK_FALSE;
    vk_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    vk_multisampling.minSampleShading = 1.0f; 
    vk_multisampling.pSampleMask = nullptr;
    vk_multisampling.alphaToCoverageEnable = VK_FALSE;
    vk_multisampling.alphaToOneEnable = VK_FALSE;

    // DEPTH STENCIL
    if (depth_state) {
        vk_depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        vk_depth_stencil.depthTestEnable = depth_state->depth_test_enable;
        vk_depth_stencil.depthWriteEnable = depth_state->depth_write_enable;
        vk_depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
        if (depth_state->stencil_test_enable) {
            vk_depth_stencil.stencilTestEnable = depth_state->stencil_test_enable;
            vk_depth_stencil.front.compareMask = 0xff;
            vk_depth_stencil.front.writeMask = 0xff;

            vk_depth_stencil.front.compareOp = (depth_state->stencil_write) ?
                VK_COMPARE_OP_ALWAYS : VK_COMPARE_OP_EQUAL;
            vk_depth_stencil.front.passOp = (depth_state->stencil_write) ?
                VK_STENCIL_OP_REPLACE : VK_STENCIL_OP_KEEP;
            vk_depth_stencil.front.failOp = VK_STENCIL_OP_KEEP;
            vk_depth_stencil.front.reference = depth_state->stencil_reference;
        }
    }

    // Color Blending
    for (int i = 0; i < blend_state->num_blend_attachments; ++i) {
        vk_blend_color_attachments[i].blendEnable =
            blend_state->blend_attachments[i].blendEnable;

        vk_blend_color_attachments[i].dstAlphaBlendFactor =
            _VulkanConvertBlendFactor(blend_state->blend_attachments[i].dstAlphaBlendFactor);
        vk_blend_color_attachments[i].dstColorBlendFactor =
            _VulkanConvertBlendFactor(blend_state->blend_attachments[i].dstColorBlendFactor);
        vk_blend_color_attachments[i].srcAlphaBlendFactor =
            _VulkanConvertBlendFactor(blend_state->blend_attachments[i].srcAlphaBlendFactor);
        vk_blend_color_attachments[i].srcColorBlendFactor =
            _VulkanConvertBlendFactor(blend_state->blend_attachments[i].srcColorBlendFactor);

        vk_blend_color_attachments[i].colorBlendOp =
            _VulkanConvertBlendOp(blend_state->blend_attachments[i].colorBlendOp);
        vk_blend_color_attachments[i].alphaBlendOp =
            _VulkanConvertBlendOp(blend_state->blend_attachments[i].alphaBlendOp);

        vk_blend_color_attachments[i].colorWriteMask = 
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    vk_color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vk_color_blending.logicOpEnable = VK_FALSE; // TODO maybe allow to configure this?
    vk_color_blending.logicOp = VK_LOGIC_OP_AND;
    vk_color_blending.attachmentCount = blend_state->num_blend_attachments;
    vk_color_blending.pAttachments = vk_blend_color_attachments.data();
    vk_color_blending.blendConstants[0] = blend_state->blend_constants[0];
    vk_color_blending.blendConstants[1] = blend_state->blend_constants[1];
    vk_color_blending.blendConstants[2] = blend_state->blend_constants[2];
    vk_color_blending.blendConstants[3] = blend_state->blend_constants[3];

    // Push constants
    vk_push_constant_range.offset = 0;
    vk_push_constant_range.size = sizeof(float)*32;
    vk_push_constant_range.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // Pipeline layout
    vk_pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vk_pipeline_layout_info.setLayoutCount = 0;
    vk_pipeline_layout_info.pSetLayouts = nullptr;
    for (int i = 0; i < num_descriptor_set_layouts; ++i) {
        vk_set_layouts[i] = descriptor_set_layouts[i]->vk_descriptor_set_layout;
        vk_pipeline_layout_info.pSetLayouts = vk_set_layouts.data();
        vk_pipeline_layout_info.setLayoutCount = num_descriptor_set_layouts;
    }
    vk_pipeline_layout_info.pushConstantRangeCount = 1;
    vk_pipeline_layout_info.pPushConstantRanges = &vk_push_constant_range;

    if (vkCreatePipelineLayout(
        vk_device,
        &vk_pipeline_layout_info,
        nullptr, &vk_pipeline_layout) != VK_SUCCESS)
    {
        // TODO  error
    }

    vk_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vk_pipeline_info.stageCount = num_shaders;
    vk_pipeline_info.pStages = vk_shader_stages.data(); // What a bad name: pStages -> shader stages active
    vk_pipeline_info.pVertexInputState = &vk_input_state;
    vk_pipeline_info.pInputAssemblyState = &vk_input_assembly;
    vk_pipeline_info.pViewportState = &vk_viewport_state;
    vk_pipeline_info.pRasterizationState = &vk_rasterizer;
    vk_pipeline_info.pMultisampleState = &vk_multisampling;
    vk_pipeline_info.pDepthStencilState = (depth_state)? &vk_depth_stencil : nullptr;
    vk_pipeline_info.pColorBlendState = &vk_color_blending;
    vk_pipeline_info.pDynamicState = &vk_dynamic_state;

    vk_pipeline_info.layout = vk_pipeline_layout;

    vk_pipeline_info.renderPass = render_pass->vk_render_pass;
    vk_pipeline_info.subpass = 0;

    /* There are actually two more parameters: basePipelineHandle and basePipelineIndex. Vulkan allows
     * you to create a new graphics pipeline by deriving from an existing pipeline. The idea of pipeline
     * derivatives is that it is less expensive to set up pipelines when they have much functionality in
     * common with an existing pipeline and switching between pipelines from the same parent can also be
     * done quicker. You can either specify the handle of an existing pipeline with basePipelineHandle
     * or reference another pipeline that is about to be created by index with basePipelineIndex. Right
     * now there is only a single pipeline, so we'll simply specify a null handle and an invalid index.
     * These values are only used if the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is also specified in
     * the flags field of VkGraphicsPipelineCreateInfo.*/
    // NOTE: MOST VENDORS DON'T RECOMMEND THIS! (GUSTAVO)
    vk_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    vk_pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(
        vk_device, VK_NULL_HANDLE, 1,
        &vk_pipeline_info, nullptr, &vk_pipeline) != VK_SUCCESS)
    {
        // TODO error
    }
}

VulkanPipeline::~VulkanPipeline()
{
    if (vk_pipeline) {
        vkDestroyPipeline(vk_device_ref, vk_pipeline, nullptr);
    }
    if (vk_pipeline_layout) {
        vkDestroyPipelineLayout(vk_device_ref, vk_pipeline_layout, nullptr);
    }

    vk_pipeline = VK_NULL_HANDLE;
    vk_pipeline_layout = VK_NULL_HANDLE;
}

}