#include "driver.h"

#include "buffer.h"
#include "texture.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "command_buffer.h"
#include "render_pipeline.h"
#include "compute_pipeline.h"
#include "descriptor_set.h"
#include "swap_chain.h"
#include "data.h"

#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    DriverVk::DriverVk(VkDevice p_device, VkPhysicalDevice p_physical_device, VkQueue p_graphics_queue,
                       VkQueue p_present_queue, VkCommandPool p_command_pool)
            : device(p_device), physical_device(p_physical_device), graphics_queue(p_graphics_queue),
              present_queue(p_present_queue), command_pool(p_command_pool) {
    }

    VkDevice DriverVk::get_device() const {
        return device;
    }

    VkQueue DriverVk::get_graphics_queue() const {
        return graphics_queue;
    }

    VkQueue DriverVk::get_present_queue() const {
        return present_queue;
    }

    VkCommandPool DriverVk::get_command_pool() const {
        return command_pool;
    }

    std::shared_ptr<DescriptorSet> DriverVk::create_descriptor_set() {
        return std::make_shared<DescriptorSetVk>();
    }

    std::shared_ptr<RenderPipeline> DriverVk::create_render_pipeline(
            const std::vector<char> &vert_source,
            const std::vector<char> &frag_source,
            const std::vector<VertexInputAttributeDescription> &p_attribute_descriptions,
            ColorBlendState blend_state,
            const std::shared_ptr<DescriptorSet> &descriptor_set,
            const std::shared_ptr<RenderPass> &render_pass) {
        auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());

        auto render_pipeline_vk = std::make_shared<RenderPipelineVk>(
                device,
                p_attribute_descriptions,
                blend_state);

        // Create descriptor set layout.
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            for (auto &pair: descriptor_set->get_descriptors()) {
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

            if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr,
                                            &render_pipeline_vk->descriptor_set_layout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor set layout!");
            }
        }

        // Create pipeline layout.
        {
            VkPipelineLayoutCreateInfo pipeline_layout_info{};
            pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_info.setLayoutCount = 1;
            pipeline_layout_info.pSetLayouts = &render_pipeline_vk->descriptor_set_layout;
            pipeline_layout_info.pushConstantRangeCount = 0;

            // Create pipeline layout.
            if (vkCreatePipelineLayout(device,
                                       &pipeline_layout_info,
                                       nullptr,
                                       &render_pipeline_vk->layout) != VK_SUCCESS) {
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

        int32_t lastBinding = -1;
        std::vector<VkVertexInputBindingDescription> binding_descriptions;
        for (auto &d: p_attribute_descriptions) {
            if (d.binding == lastBinding) continue;
            lastBinding = d.binding;

            VkVertexInputBindingDescription binding_description{};
            binding_description.binding = d.binding;
            binding_description.stride = d.stride;
            binding_description.inputRate = d.vertex_input_rate == VertexInputRate::VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX
                                                                                           : VK_VERTEX_INPUT_RATE_INSTANCE;

            binding_descriptions.push_back(binding_description);
        }

        std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        uint32_t location = 0;
        for (auto &d: p_attribute_descriptions) {
            VkVertexInputAttributeDescription attribute_description{};
            attribute_description.binding = d.binding;
            attribute_description.location = location++;
            attribute_description.format = to_vk_format(d.type, d.size);
            attribute_description.offset = d.offset;

            attribute_descriptions.push_back(attribute_description);
        }

        vertex_input_info.vertexBindingDescriptionCount = binding_descriptions.size();
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
        vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
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
        //viewportState.pViewports = &viewport; // We will have dynamic viewport size.
        viewport_state.scissorCount = 1;
        //viewportState.pScissors = &scissor; // We will have dynamic scissor size.

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
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_TRUE;
        // Need to set blend config if blend is enabled.
        {
            color_blend_attachment.srcColorBlendFactor = to_vk_blend_factor(blend_state.src_blend_factor);
            color_blend_attachment.dstColorBlendFactor = to_vk_blend_factor(blend_state.dst_blend_factor);
            color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_attachment.srcAlphaBlendFactor = to_vk_blend_factor(blend_state.src_blend_factor);
            color_blend_attachment.dstAlphaBlendFactor = to_vk_blend_factor(blend_state.dst_blend_factor);
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

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shader_stages;
        pipelineInfo.pVertexInputState = &vertex_input_info;
        pipelineInfo.pInputAssemblyState = &input_assembly;
        pipelineInfo.pViewportState = &viewport_state;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &color_blending;
        pipelineInfo.layout = render_pipeline_vk->layout;
        pipelineInfo.renderPass = render_pass_vk->vk_render_pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState = &dynamic; // Make viewport and scissor dynamic.

        // Create pipeline.
        if (vkCreateGraphicsPipelines(device,
                                      VK_NULL_HANDLE,
                                      1,
                                      &pipelineInfo,
                                      nullptr,
                                      &render_pipeline_vk->vk_pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }

        // Clean up shader modules.
        vkDestroyShaderModule(device, vert_shader_module, nullptr);
        vkDestroyShaderModule(device, frag_shader_module, nullptr);

        return render_pipeline_vk;
    }

    std::shared_ptr<ComputePipeline> DriverVk::create_compute_pipeline(
            const std::vector<char> &comp_source,
            const std::shared_ptr<DescriptorSet> &descriptor_set) {
        auto compute_pipeline_vk = std::make_shared<ComputePipelineVk>(device);

        // Create descriptor set layout.
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            for (auto &pair: descriptor_set->get_descriptors()) {
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

            if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr,
                                            &compute_pipeline_vk->descriptor_set_layout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor set layout!");
            }
        }

        // Create pipeline layout.
        {
            VkPipelineLayoutCreateInfo pipeline_layout_info{};
            pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_info.setLayoutCount = 1;
            pipeline_layout_info.pSetLayouts = &compute_pipeline_vk->descriptor_set_layout;
            pipeline_layout_info.pushConstantRangeCount = 0;

            // Create pipeline layout.
            if (vkCreatePipelineLayout(device,
                                       &pipeline_layout_info,
                                       nullptr,
                                       &compute_pipeline_vk->layout) != VK_SUCCESS) {
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
        pipeline_create_info.layout = compute_pipeline_vk->layout;
        pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_create_info.basePipelineIndex = 0;

        // Create pipeline.
        if (vkCreateComputePipelines(device,
                                     VK_NULL_HANDLE,
                                     1,
                                     &pipeline_create_info,
                                     nullptr,
                                     &compute_pipeline_vk->vk_pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute pipeline!");
        }

        // Clean up shader module.
        vkDestroyShaderModule(device, comp_shader_module, nullptr);

        return compute_pipeline_vk;
    }

    std::shared_ptr<RenderPass> DriverVk::create_render_pass(
            TextureFormat format,
            AttachmentLoadOp load_op,
            TextureLayout final_layout) {
        auto render_pass_vk = std::make_shared<RenderPassVk>(device, format, load_op, final_layout);

        return render_pass_vk;
    }

    std::shared_ptr<Framebuffer> DriverVk::create_framebuffer(
            const std::shared_ptr<RenderPass> &render_pass,
            const std::shared_ptr<Texture> &texture) {
        auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());

        auto framebuffer_vk = std::make_shared<FramebufferVk>(
                device,
                render_pass_vk->vk_render_pass,
                texture);

        return framebuffer_vk;
    }

    std::shared_ptr<Buffer> DriverVk::create_buffer(BufferType type, size_t size, MemoryProperty property) {
        auto buffer_vk = std::make_shared<BufferVk>(device, type, size, property);

        auto vk_memory_property = to_vk_memory_property(property);

        switch (type) {
            case BufferType::Uniform: {
                create_vk_buffer(size,
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 vk_memory_property,
                                 buffer_vk->vk_buffer,
                                 buffer_vk->vk_device_memory);
            }
                break;
            case BufferType::Vertex: {
                create_vk_buffer(size,
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                 vk_memory_property,
                                 buffer_vk->vk_buffer,
                                 buffer_vk->vk_device_memory);
            }
                break;
            case BufferType::Storage: {
                create_vk_buffer(size,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                 vk_memory_property,
                                 buffer_vk->vk_buffer,
                                 buffer_vk->vk_device_memory);
            }
                break;
        }

        return buffer_vk;
    }

    std::shared_ptr<Texture> DriverVk::create_texture(uint32_t width, uint32_t height, TextureFormat format) {
        auto texture_vk = std::make_shared<TextureVk>(
                device,
                width,
                height,
                format
        );

        create_vk_image(width,
                        height,
                        to_vk_texture_format(format),
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        texture_vk->image,
                        texture_vk->image_memory
        );

        // Create image view.
        texture_vk->image_view = create_vk_image_view(texture_vk->image,
                                                      to_vk_texture_format(format),
                                                      VK_IMAGE_ASPECT_COLOR_BIT);

        // Create sampler.
        texture_vk->sampler = create_vk_sampler();

        return texture_vk;
    }

    std::shared_ptr<CommandBuffer> DriverVk::create_command_buffer(bool one_time) {
        // Allocate a command buffer.
        // ----------------------------------------
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);
        // ----------------------------------------

        auto command_buffer_vk = std::make_shared<CommandBufferVk>(command_buffer, device);
        command_buffer_vk->one_time = one_time;

        return command_buffer_vk;
    }

    VkShaderModule DriverVk::create_shader_module(const std::vector<char> &code) {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shader_module;
        if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }

        return shader_module;
    }

    void DriverVk::create_vk_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                                   VkDeviceMemory &image_memory) const {
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

        if (vkCreateImage(device, &image_info, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image!");
        }

        // Allocating memory for an image.
        // -------------------------------------
        VkMemoryRequirements mem_requirements;
        // Returns the memory requirements for specified Vulkan object.
        vkGetImageMemoryRequirements(device, image, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, image_memory, 0);
        // -------------------------------------
    }

    VkImageView DriverVk::create_vk_image_view(VkImage image,
                                               VkFormat format,
                                               VkImageAspectFlags aspect_flags) const {
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
        if (vkCreateImageView(device, &view_info, nullptr, &image_view) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view!");
        }

        return image_view;
    }

    VkSampler DriverVk::create_vk_sampler() const {
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

        // The borderColor field specifies which color is returned when sampling beyond the image with clamp to border addressing mode.
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
        if (vkCreateSampler(device, &sampler_info, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture sampler!");
        }

        return sampler;
    }

    void DriverVk::create_vk_buffer(VkDeviceSize size,
                                    VkBufferUsageFlags usage,
                                    VkMemoryPropertyFlags properties,
                                    VkBuffer &buffer,
                                    VkDeviceMemory &buffer_memory) {
        // Structure specifying the parameters of a newly created buffer object.
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size; // Size in bytes of the buffer to be created.
        buffer_info.usage = usage; // Specifying allowed usages of the buffer.
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Specifying the sharing mode of the buffer when it will be accessed by multiple queue families.

        // Allocate GPU buffer.
        if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

        // Structure containing parameters of a memory allocation.
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits,
                                                      properties); // Index identifying a memory type.

        // Allocate device memory.
        if (vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate device memory!");
        }

        // Bind the buffer and memory.
        vkBindBufferMemory(device, buffer, buffer_memory, 0);
    }

    void DriverVk::create_vk_render_pass(VkFormat format, VkRenderPass &render_pass) {
        // Color attachment.
        // ----------------------------------------
        VkAttachmentDescription color_attachment{};
        color_attachment.format = format; // Specifying the format of the image view that will be used for the attachment.
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // Specifying the number of samples of the image.
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Specifying how the contents of color and depth components of the attachment are treated at the beginning of the subpass where it is first used.
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Specifying how the contents of color and depth components of the attachment are treated at the end of the subpass where it is last used.
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // The layout the attachment image subresource will be in when a render pass instance begins.
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // The layout the attachment image subresource will be transitioned to when a render pass instance ends.

        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifying the layout the attachment uses during the subpass.
        // ----------------------------------------

        // Depth attachment.
        // ----------------------------------------
        VkAttachmentDescription depth_attachment{};
        depth_attachment.format = find_depth_format();
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        // ----------------------------------------

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        // Use sub-pass dependencies for layout transitions.
        // ----------------------------------------
        std::array<VkSubpassDependency, 2> dependencies{};

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
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
        // ----------------------------------------

        std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};

        // Create the actual render pass.
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
        render_pass_info.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    uint32_t DriverVk::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties mem_properties;

        // Reports memory information for the specified physical device.
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    VkFormat DriverVk::find_supported_format(const std::vector<VkFormat> &candidates,
                                             VkImageTiling tiling,
                                             VkFormatFeatureFlags features) const {
        for (VkFormat format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    VkFormat DriverVk::find_depth_format() const {
        return find_supported_format(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void DriverVk::copy_vk_buffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer,
                                  VkDeviceSize size) const {
        // Send copy command.
        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = size;

        vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
    }

    void DriverVk::copy_buffer_to_image(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image,
                                        uint32_t width, uint32_t height) const {
        // Structure specifying a buffer image copy operation.
        VkBufferImageCopy region{};
        region.bufferOffset = 0; // Offset in bytes from the start of the buffer object where the image data is copied from or to.
        region.bufferRowLength = 0; // Specify in texels a subregion of a larger two- or three-dimensional image in buffer memory, and control the addressing calculations.
        region.bufferImageHeight = 0;

        // A VkImageSubresourceLayers used to specify the specific image subresources of the image used for the source or destination image data.
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        // Selects the initial x, y, z offsets in texels of the subregion of the source or destination image data.
        region.imageOffset = {0, 0, 0};
        // Size in texels of the image to copy in width, height and depth.
        region.imageExtent = {width, height, 1};

        // Copy data from a buffer into an image.
        vkCmdCopyBufferToImage(
                command_buffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
        );
    }

    void DriverVk::copy_data_to_memory(const void *src, VkDeviceMemory buffer_memory, size_t data_size) const {
        void *data;
        vkMapMemory(device, buffer_memory, 0, data_size, 0, &data);
        memcpy(data, src, data_size);
        vkUnmapMemory(device, buffer_memory);
    }

    void DriverVk::copy_data_from_memory(void *dst, VkDeviceMemory buffer_memory, size_t data_size) const {
        void *data;
        vkMapMemory(device, buffer_memory, 0, data_size, 0, &data);
        memcpy(dst, data, data_size);
        vkUnmapMemory(device, buffer_memory);
    }
}

#endif
