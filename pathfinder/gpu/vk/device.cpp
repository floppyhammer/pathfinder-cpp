#include "device.h"

#include <memory>

#include "base.h"
#include "buffer.h"
#include "command_encoder.h"
#include "compute_pipeline.h"
#include "debug_marker.h"
#include "descriptor_set.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "swap_chain.h"
#include "texture.h"

namespace Pathfinder {

DeviceVk::DeviceVk(VkDevice vk_device,
                   VkPhysicalDevice vk_physical_device,
                   VkQueue vk_graphics_queue,
                   VkQueue vk_present_queue,
                   VkCommandPool vk_command_pool)
    : vk_device_(vk_device), vk_physical_device_(vk_physical_device), vk_graphics_queue_(vk_graphics_queue),
      vk_present_queue_(vk_present_queue), vk_command_pool_(vk_command_pool) {}

VkDevice DeviceVk::get_device() const {
    return vk_device_;
}

VkPhysicalDevice DeviceVk::get_physical_device() const {
    return vk_physical_device_;
}

VkQueue DeviceVk::get_graphics_queue() const {
    return vk_graphics_queue_;
}

VkQueue DeviceVk::get_present_queue() const {
    return vk_present_queue_;
}

VkCommandPool DeviceVk::get_command_pool() const {
    return vk_command_pool_;
}

std::shared_ptr<DescriptorSet> DeviceVk::create_descriptor_set() {
    return std::shared_ptr<DescriptorSetVk>(new DescriptorSetVk());
}

std::shared_ptr<RenderPipeline> DeviceVk::create_render_pipeline(
    const std::vector<char> &vert_source,
    const std::vector<char> &frag_source,
    const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
    BlendState blend_state,
    const std::shared_ptr<DescriptorSet> &descriptor_set,
    const std::shared_ptr<RenderPass> &render_pass,
    const std::string &label) {
    auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());

    auto render_pipeline_vk =
        std::shared_ptr<RenderPipelineVk>(new RenderPipelineVk(vk_device_, attribute_descriptions, blend_state, label));

    // Create descriptor set layout.
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        for (auto &pair : descriptor_set->get_descriptors()) {
            auto &d = pair.second;

            VkDescriptorSetLayoutBinding binding;
            binding.binding = d.binding;
            binding.descriptorCount = 1;
            binding.descriptorType = to_vk_descriptor_type(d.type);
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = to_vk_shader_stage(d.stage);

            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = bindings.size();
        layout_info.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(vk_device_,
                                        &layout_info,
                                        nullptr,
                                        &render_pipeline_vk->vk_descriptor_set_layout_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }
    }

    // Create pipeline layout.
    {
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &render_pipeline_vk->vk_descriptor_set_layout_;
        pipeline_layout_info.pushConstantRangeCount = 0;

        // Create pipeline layout.
        if (vkCreatePipelineLayout(vk_device_, &pipeline_layout_info, nullptr, &render_pipeline_vk->vk_layout_) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }

    VkShaderModule vert_shader_module = create_shader_module(vert_source);
    VkShaderModule frag_shader_module = create_shader_module(frag_source);

    // Specify shader stages.
    VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    // Set up how to accept vertex data.
    // -----------------------------------------------------
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    int32_t last_binding = -1;
    std::vector<VkVertexInputBindingDescription> binding_descriptions;
    for (auto &d : attribute_descriptions) {
        if (d.binding == last_binding) {
            continue;
        }
        last_binding = d.binding;

        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = d.binding;
        binding_description.stride = d.stride;
        binding_description.inputRate = d.vertex_input_rate == VertexInputRate::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX
                                                                                       : VK_VERTEX_INPUT_RATE_INSTANCE;

        binding_descriptions.push_back(binding_description);
    }

    std::vector<VkVertexInputAttributeDescription> vk_attribute_descriptions;
    uint32_t location = 0;
    for (auto &d : attribute_descriptions) {
        VkVertexInputAttributeDescription attribute_description{};
        attribute_description.binding = d.binding;
        attribute_description.location = location++;
        attribute_description.format = to_vk_format(d.type, d.size);
        attribute_description.offset = d.offset;

        vk_attribute_descriptions.push_back(attribute_description);
    }

    vertex_input_info.vertexBindingDescriptionCount = binding_descriptions.size();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vk_attribute_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
    vertex_input_info.pVertexAttributeDescriptions = vk_attribute_descriptions.data();
    // -----------------------------------------------------

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    // Specify that these states will be dynamic, i.e. not part of pipeline state object.
    std::array<VkDynamicState, 2> dynamics{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic.pDynamicStates = dynamics.data();
    dynamic.dynamicStateCount = dynamics.size();

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    // viewportState.pViewports = &viewport; // We will have dynamic viewport size.
    viewport_state.scissorCount = 1;
    // viewportState.pScissors = &scissor; // We will have dynamic scissor size.

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    color_blend_attachment.blendEnable = VK_FALSE;

    if (blend_state.enabled) {
        color_blend_attachment.blendEnable = VK_TRUE;

        // Need to set blend config if blend is enabled.
        color_blend_attachment.srcColorBlendFactor = to_vk_blend_factor(blend_state.color.src_factor);
        color_blend_attachment.dstColorBlendFactor = to_vk_blend_factor(blend_state.color.dst_factor);
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = to_vk_blend_factor(blend_state.alpha.src_factor);
        color_blend_attachment.dstAlphaBlendFactor = to_vk_blend_factor(blend_state.alpha.dst_factor);
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = render_pipeline_vk->vk_layout_;
    pipeline_info.renderPass = render_pass_vk->vk_render_pass_;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.pDynamicState = &dynamic; // Make viewport and scissor dynamic.

    // Create pipeline.
    if (vkCreateGraphicsPipelines(vk_device_,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipeline_info,
                                  nullptr,
                                  &render_pipeline_vk->vk_pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    // Clean up shader modules.
    vkDestroyShaderModule(vk_device_, vert_shader_module, nullptr);
    vkDestroyShaderModule(vk_device_, frag_shader_module, nullptr);

    DebugMarker::get_singleton()->set_object_name(vk_device_,
                                                  (uint64_t)render_pipeline_vk->vk_pipeline_,
                                                  VK_OBJECT_TYPE_PIPELINE,
                                                  label);

    return render_pipeline_vk;
}

std::shared_ptr<ComputePipeline> DeviceVk::create_compute_pipeline(const std::vector<char> &comp_source,
                                                                   const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                                   const std::string &label) {
    auto compute_pipeline_vk = std::shared_ptr<ComputePipelineVk>(new ComputePipelineVk(vk_device_, label));

    // Create descriptor set layout.
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        for (auto &pair : descriptor_set->get_descriptors()) {
            auto &d = pair.second;

            VkDescriptorSetLayoutBinding binding;
            binding.binding = d.binding;
            binding.descriptorCount = 1;
            binding.descriptorType = to_vk_descriptor_type(d.type);
            binding.pImmutableSamplers = nullptr;
            binding.stageFlags = to_vk_shader_stage(d.stage);

            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = bindings.size();
        layout_info.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(vk_device_,
                                        &layout_info,
                                        nullptr,
                                        &compute_pipeline_vk->vk_descriptor_set_layout_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }
    }

    // Create pipeline layout.
    {
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &compute_pipeline_vk->vk_descriptor_set_layout_;
        pipeline_layout_info.pushConstantRangeCount = 0;

        // Create pipeline layout.
        if (vkCreatePipelineLayout(vk_device_, &pipeline_layout_info, nullptr, &compute_pipeline_vk->vk_layout_) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute pipeline layout!");
        }
    }

    VkShaderModule comp_shader_module = create_shader_module(comp_source);

    // Specify shader stages.
    VkPipelineShaderStageCreateInfo comp_shader_stage_info{};
    comp_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    comp_shader_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    comp_shader_stage_info.module = comp_shader_module;
    comp_shader_stage_info.pName = "main";

    VkComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_create_info.pNext = nullptr;
    pipeline_create_info.flags = 0;
    pipeline_create_info.stage = comp_shader_stage_info;
    pipeline_create_info.layout = compute_pipeline_vk->vk_layout_;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = 0;

    // Create pipeline.
    if (vkCreateComputePipelines(vk_device_,
                                 VK_NULL_HANDLE,
                                 1,
                                 &pipeline_create_info,
                                 nullptr,
                                 &compute_pipeline_vk->vk_pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    // Clean up shader module.
    vkDestroyShaderModule(vk_device_, comp_shader_module, nullptr);

    DebugMarker::get_singleton()->set_object_name(vk_device_,
                                                  (uint64_t)compute_pipeline_vk->vk_pipeline_,
                                                  VK_OBJECT_TYPE_PIPELINE,
                                                  label);

    return compute_pipeline_vk;
}

std::shared_ptr<RenderPass> DeviceVk::create_render_pass(TextureFormat format,
                                                         AttachmentLoadOp load_op,
                                                         const std::string &label) {
    return std::shared_ptr<RenderPassVk>(new RenderPassVk(vk_device_, format, load_op, false, label));
}

std::shared_ptr<RenderPass> DeviceVk::create_swap_chain_render_pass(TextureFormat format, AttachmentLoadOp load_op) {
    return std::shared_ptr<RenderPassVk>(new RenderPassVk(vk_device_, format, load_op, true, "Swap chain render pass"));
}

std::shared_ptr<Framebuffer> DeviceVk::create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                          const std::shared_ptr<Texture> &texture,
                                                          const std::string &label) {
    auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());

    auto framebuffer_vk =
        std::shared_ptr<FramebufferVk>(new FramebufferVk(vk_device_, render_pass_vk->vk_render_pass_, texture));

    framebuffer_vk->set_label(label);

    return framebuffer_vk;
}

std::shared_ptr<Buffer> DeviceVk::create_buffer(const BufferDescriptor &desc, const std::string &label) {
    auto buffer_vk = std::shared_ptr<BufferVk>(new BufferVk(vk_device_, desc));

    auto vk_memory_property = to_vk_memory_property(desc.property);

    switch (desc.type) {
        case BufferType::Uniform: {
            create_vk_buffer(desc.size,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             vk_memory_property,
                             buffer_vk->vk_buffer_,
                             buffer_vk->vk_device_memory_);
        } break;
        case BufferType::Vertex: {
            create_vk_buffer(desc.size,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             vk_memory_property,
                             buffer_vk->vk_buffer_,
                             buffer_vk->vk_device_memory_);
        } break;
        case BufferType::Storage: {
            create_vk_buffer(desc.size,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                             vk_memory_property,
                             buffer_vk->vk_buffer_,
                             buffer_vk->vk_device_memory_);
        } break;
        default:
            abort();
    }

    buffer_vk->set_label(label);

    return buffer_vk;
}

std::shared_ptr<Texture> DeviceVk::create_texture(const TextureDescriptor &desc, const std::string &label) {
    auto texture_vk = std::shared_ptr<TextureVk>(new TextureVk(vk_device_, desc));

    create_vk_image(desc.size.x,
                    desc.size.y,
                    to_vk_texture_format(desc.format),
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                        VK_IMAGE_USAGE_STORAGE_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    texture_vk->vk_image_,
                    texture_vk->vk_image_memory_,
                    texture_vk->memory_size_);

    // Create image view.
    texture_vk->vk_image_view_ =
        create_vk_image_view(texture_vk->vk_image_, to_vk_texture_format(desc.format), VK_IMAGE_ASPECT_COLOR_BIT);

    texture_vk->set_label(label);

    return texture_vk;
}

std::shared_ptr<Sampler> DeviceVk::create_sampler(SamplerDescriptor descriptor) {
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = to_vk_sampler_filter(descriptor.mag_filter);
    sampler_info.minFilter = to_vk_sampler_filter(descriptor.min_filter);

    // Noteable: It has to be CLAMP_TO_EDGE. Artifacts will show for both REPEAT and MIRRORED_REPEAT.
    sampler_info.addressModeU = to_vk_sampler_address_mode(descriptor.address_mode_u);
    sampler_info.addressModeV = to_vk_sampler_address_mode(descriptor.address_mode_v);
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // Has to be disabled to prevent artifacts.
    sampler_info.anisotropyEnable = VK_FALSE;

    // The borderColor field specifies which color is returned when sampling beyond the image with clamp to border
    // addressing mode.
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    sampler_info.unnormalizedCoordinates = VK_FALSE;

    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    // All of these fields apply to mipmapping.
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    VkSampler sampler;
    if (vkCreateSampler(vk_device_, &sampler_info, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler!");
    }

    return std::shared_ptr<SamplerVk>(new SamplerVk(descriptor, sampler, vk_device_));
}

std::shared_ptr<CommandEncoder> DeviceVk::create_command_encoder(const std::string &label) {
    // Allocate a command buffer.
    // ----------------------------------------
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = vk_command_pool_;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(vk_device_, &alloc_info, &command_buffer);
    // ----------------------------------------

    auto command_encoder_vk = std::shared_ptr<CommandEncoderVk>(new CommandEncoderVk(command_buffer, this));
    command_encoder_vk->label_ = label;

    return command_encoder_vk;
}

VkShaderModule DeviceVk::create_shader_module(const std::vector<char> &code) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(vk_device_, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shader_module;
}

void DeviceVk::create_vk_image(uint32_t width,
                               uint32_t height,
                               VkFormat format,
                               VkImageTiling tiling,
                               VkImageUsageFlags usage,
                               VkMemoryPropertyFlags properties,
                               VkImage &image,
                               VkDeviceMemory &image_memory,
                               size_t &memory_size) const {
    // Create image.
    // -------------------------------------
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vk_device_, &image_info, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image!");
    }
    // -------------------------------------

    // Create image memory.
    // -------------------------------------
    // Get the memory requirements for the image.
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(vk_device_, image, &mem_requirements);

    memory_size = mem_requirements.size;

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vk_device_, &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate image memory!");
    }
    // -------------------------------------

    // Bind the image and memory.
    vkBindImageMemory(vk_device_, image, image_memory, 0);
}

VkImageView DeviceVk::create_vk_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) const {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (vkCreateImageView(vk_device_, &view_info, nullptr, &image_view) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image view!");
    }

    return image_view;
}

VkSampler DeviceVk::create_vk_sampler() const {
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;

    // Noteable: It has to be CLAMP_TO_EDGE. Artifacts will show for both REPEAT and MIRRORED_REPEAT.
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    // Has to be disabled to prevent artifacts.
    sampler_info.anisotropyEnable = VK_FALSE;

    // The borderColor field specifies which color is returned when sampling beyond the image with clamp to border
    // addressing mode.
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    sampler_info.unnormalizedCoordinates = VK_FALSE;

    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    // All of these fields apply to mipmapping.
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    VkSampler sampler;
    if (vkCreateSampler(vk_device_, &sampler_info, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler!");
    }

    return sampler;
}

void DeviceVk::create_vk_buffer(VkDeviceSize size,
                                VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer &buffer,
                                VkDeviceMemory &buffer_memory) {
    // Structure specifying the parameters of a newly created buffer object.
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;                             // Size in bytes of the buffer to be created.
    buffer_info.usage = usage;                           // Specifying allowed usages of the buffer.
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Specifying the sharing mode of the buffer when it will be
                                                         // accessed by multiple queue families.

    // Create buffer.
    if (vkCreateBuffer(vk_device_, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    // Allocate buffer memory.
    // -------------------------------------
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(vk_device_, buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits,
                                                  properties); // Index identifying a memory type.

    if (vkAllocateMemory(vk_device_, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }
    // -------------------------------------

    // Bind the buffer and memory.
    vkBindBufferMemory(vk_device_, buffer, buffer_memory, 0);
}

uint32_t DeviceVk::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties mem_properties;

    // Reports memory information for the specified physical device.
    vkGetPhysicalDeviceMemoryProperties(vk_physical_device_, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

VkFormat DeviceVk::find_supported_format(const std::vector<VkFormat> &candidates,
                                         VkImageTiling tiling,
                                         VkFormatFeatureFlags features) const {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(vk_physical_device_, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format!");
}

VkFormat DeviceVk::find_depth_format() const {
    return find_supported_format({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void DeviceVk::copy_vk_buffer(VkCommandBuffer cmd_buffer,
                              VkBuffer src_buffer,
                              VkBuffer dst_buffer,
                              VkDeviceSize size,
                              VkDeviceSize src_offset,
                              VkDeviceSize dst_offset) const {
    // Send copy command.
    VkBufferCopy copy_region{};
    copy_region.srcOffset = src_offset;
    copy_region.dstOffset = dst_offset;
    copy_region.size = size;

    vkCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, &copy_region);
}

void DeviceVk::copy_buffer_to_image(VkCommandBuffer cmd_buffer,
                                    VkBuffer buffer,
                                    VkImage image,
                                    uint32_t width,
                                    uint32_t height) const {
    // Structure specifying a buffer image copy operation.
    VkBufferImageCopy region{};
    region.bufferOffset =
        0; // Offset in bytes from the start of the buffer object where the image data is copied from or to.
    region.bufferRowLength = 0; // Specify in texels a subregion of a larger two- or three-dimensional image in buffer
                                // memory, and control the addressing calculations.
    region.bufferImageHeight = 0;

    // A VkImageSubresourceLayers used to specify the specific image subresources of the image used for the source or
    // destination image data.
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    // Selects the initial x, y, z offsets in texels of the subregion of the source or destination image data.
    region.imageOffset = {0, 0, 0};
    // Size in texels of the image to copy in width, height and depth.
    region.imageExtent = {width, height, 1};

    // Copy data from a buffer into an image.
    vkCmdCopyBufferToImage(cmd_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void DeviceVk::copy_data_to_mappable_memory(const void *src, VkDeviceMemory buffer_memory, size_t data_size) const {
    void *data;
    vkMapMemory(vk_device_, buffer_memory, 0, data_size, 0, &data);
    memcpy(data, src, data_size);
    vkUnmapMemory(vk_device_, buffer_memory);
}

void DeviceVk::copy_data_from_mappable_memory(void *dst, VkDeviceMemory buffer_memory, size_t data_size) const {
    void *data;
    vkMapMemory(vk_device_, buffer_memory, 0, data_size, 0, &data);
    memcpy(dst, data, data_size);
    vkUnmapMemory(vk_device_, buffer_memory);
}

} // namespace Pathfinder
