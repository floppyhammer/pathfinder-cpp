#include "driver.h"

#include "buffer.h"
#include "texture.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "command_buffer.h"
#include "render_pipeline.h"
#include "descriptor_set.h"
#include "data.h"

#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    DriverVk::DriverVk(VkDevice p_device, VkPhysicalDevice p_physical_device, VkQueue p_graphics_queue,
                       VkQueue p_present_queue, VkCommandPool p_command_pool)
            : device(p_device), physicalDevice(p_physical_device), graphicsQueue(p_graphics_queue),
            presentQueue(p_present_queue), commandPool(p_command_pool) {
    }

    VkDevice DriverVk::get_device() const {
        return device;
    }

    VkQueue DriverVk::get_graphics_queue() const {
        return graphicsQueue;
    }

    VkQueue DriverVk::get_present_queue() const {
        return presentQueue;
    }

    VkCommandPool DriverVk::get_command_pool() const {
        return commandPool;
    }

    std::shared_ptr<SwapChain> DriverVk::create_swap_chain(uint32_t p_width, uint32_t p_height) {
        return std::shared_ptr<SwapChain>();
    }

    std::shared_ptr<DescriptorSet> DriverVk::create_descriptor_set() {
        return std::make_shared<DescriptorSetVk>();
    }

    std::shared_ptr<RenderPipeline> DriverVk::create_render_pipeline(
            const std::vector<char> &vert_source,
            const std::vector<char> &frag_source,
            const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
            ColorBlendState blend_state,
            const std::shared_ptr<DescriptorSet> &descriptor_set,
            const std::shared_ptr<RenderPass> &render_pass) {
        auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());
        auto render_pipeline_vk = std::make_shared<RenderPipelineVk>(device, attribute_descriptions, blend_state);

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

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = bindings.size();
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                            &render_pipeline_vk->descriptor_set_layout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor set layout!");
            }
        }

        // Create pipeline layout.
        {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &render_pipeline_vk->descriptor_set_layout;
            pipelineLayoutInfo.pushConstantRangeCount = 0;

            // Create pipeline layout.
            if (vkCreatePipelineLayout(device,
                                       &pipelineLayoutInfo,
                                       nullptr,
                                       &render_pipeline_vk->layout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create pipeline layout!");
            }
        }

        VkShaderModule vertShaderModule = createShaderModule(vert_source);
        VkShaderModule fragShaderModule = createShaderModule(frag_source);

        // Specify shader stages.
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        // Set up how to accept vertex data.
        // -----------------------------------------------------
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        int32_t lastBinding = -1;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        for (auto &d: attribute_descriptions) {
            if (d.binding == lastBinding) continue;
            lastBinding = d.binding;

            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = d.binding;
            bindingDescription.stride = d.stride;
            bindingDescription.inputRate = d.vertex_input_rate == VertexInputRate::VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX
                                                                                          : VK_VERTEX_INPUT_RATE_INSTANCE;
            bindingDescriptions.push_back(bindingDescription);
        }

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        uint32_t location = 0;
        for (auto &d: attribute_descriptions) {
            VkVertexInputAttributeDescription attributeDescription{};
            attributeDescription.binding = d.binding;
            attributeDescription.location = location++;
            attributeDescription.format = to_vk_format(d.type, d.size);
            attributeDescription.offset = d.offset;

            attributeDescriptions.push_back(attributeDescription);
        }

        vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        // -----------------------------------------------------

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) render_pass_vk->extent.x;
        viewport.height = (float) render_pass_vk->extent.y;
        viewport.minDepth = 0.0f; // The depth range for the viewport.
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {render_pass_vk->extent.x, render_pass_vk->extent.y};

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        // FIXME
        // Need to set blend config if blend is enabled.
        {
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = render_pipeline_vk->layout;
        pipelineInfo.renderPass = render_pass_vk->id;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;

        // Create pipeline.
        if (vkCreateGraphicsPipelines(device,
                                      VK_NULL_HANDLE,
                                      1,
                                      &pipelineInfo,
                                      nullptr,
                                      &render_pipeline_vk->id) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }

        // Clean up shader modules.
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);

        return render_pipeline_vk;
    }

    std::shared_ptr<ComputePipeline> DriverVk::create_compute_pipeline(const std::vector<char> &comp_shader_code,
                                                                       const std::shared_ptr<DescriptorSet> &descriptor_set) {
        return std::make_shared<ComputePipeline>();
    }

    std::shared_ptr<RenderPass> DriverVk::create_render_pass(TextureFormat format) {
        auto render_pass_vk = std::make_shared<RenderPassVk>();

        // Color attachment.
        // ----------------------------------------
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = to_vk_texture_format(format); // Specifying the format of the image view that will be used for the attachment.
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Specifying the number of samples of the image.
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Specifying how the contents of color and depth components of the attachment are treated at the beginning of the subpass where it is first used.
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Specifying how the contents of color and depth components of the attachment are treated at the end of the subpass where it is last used.
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // The layout the attachment image subresource will be in when a render pass instance begins.
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // The layout the attachment image subresource will be transitioned to when a render pass instance ends.

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifying the layout the attachment uses during the subpass.
        // ----------------------------------------

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        // Use subpass dependencies for layout transitions.
        // ----------------------------------------
        std::array<VkSubpassDependency, 1> dependencies{};

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};

        // Create the actual render pass.
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &render_pass_vk->id) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }

        return render_pass_vk;
    }

    std::shared_ptr<Framebuffer> DriverVk::create_framebuffer(uint32_t width, uint32_t height,
                                                              TextureFormat format, DataType type,
                                                              const std::shared_ptr<RenderPass> &render_pass) {
        auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());

        auto framebuffer_vk = std::make_shared<FramebufferVk>(
                width, height, format, type);

        // Color attachment.
        {
            // Color texture.
            auto texture_vk = std::make_shared<TextureVk>(
                    device, width, height, format, type);

            framebuffer_vk->texture = texture_vk;

            createVkImage(width,
                          height,
                          to_vk_texture_format(format),
                          VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                          texture_vk->image,
                          texture_vk->image_memory);

            // Create image view.
            texture_vk->image_view = createVkImageView(texture_vk->image,
                                                       to_vk_texture_format(format),
                                                       VK_IMAGE_ASPECT_COLOR_BIT);

            // Create sampler.
            createVkTextureSampler(texture_vk->sampler);
        }

        // Create framebuffer.
        {
            std::array<VkImageView, 1> attachments = {framebuffer_vk->texture->image_view};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = render_pass_vk->id;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device,
                                    &framebufferInfo,
                                    nullptr,
                                    &framebuffer_vk->framebuffer_id) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }

        // Fill a descriptor for later use in a descriptor set.
        framebuffer_vk->descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        framebuffer_vk->descriptor.imageView = framebuffer_vk->texture->image_view;
        framebuffer_vk->descriptor.sampler = framebuffer_vk->texture->sampler;

        return framebuffer_vk;
    }

    std::shared_ptr<Buffer> DriverVk::create_buffer(BufferType type, size_t size) {
        auto buffer_vk = std::make_shared<BufferVk>(device, type, size);

        switch (type) {
            case BufferType::Uniform: {
                createVkBuffer(size,
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               buffer_vk->id,
                               buffer_vk->device_memory);
            }
                break;
            case BufferType::Vertex: {
                createVkBuffer(size,
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               buffer_vk->id,
                               buffer_vk->device_memory);
            }
                break;
            case BufferType::General:
                break;
        }

        return buffer_vk;
    }

    std::shared_ptr<Texture> DriverVk::create_texture(uint32_t width, uint32_t height,
                                                      TextureFormat format, DataType type) {
        auto texture_vk = std::make_shared<TextureVk>(
                device, width, height, format, type);

        createVkImage(width,
                      height,
                      to_vk_texture_format(format),
                      VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      texture_vk->image,
                      texture_vk->image_memory);

        // Create image view.
        texture_vk->image_view = createVkImageView(texture_vk->image,
                                                   to_vk_texture_format(format),
                                                   VK_IMAGE_ASPECT_COLOR_BIT);

        // Create sampler.
        createVkTextureSampler(texture_vk->sampler);

        return texture_vk;
    }

    std::shared_ptr<CommandBuffer> DriverVk::create_command_buffer() {
        // Allocate a command buffer.
        // ----------------------------------------
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
        // ----------------------------------------

        auto command_buffer_vk = std::make_shared<CommandBufferVk>();
        command_buffer_vk->vk_command_buffer = commandBuffer;

        return command_buffer_vk;
    }

    VkShaderModule DriverVk::createShaderModule(const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shader_module;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shader_module) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }

        return shader_module;
    }

    void DriverVk::createVkImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                                 VkDeviceMemory &imageMemory) const {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image!");
        }

        // Allocating memory for an image.
        // -------------------------------------
        VkMemoryRequirements memRequirements;
        // Returns the memory requirements for specified Vulkan object.
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
        // -------------------------------------
    }

    VkImageView DriverVk::createVkImageView(VkImage image,
                                            VkFormat format,
                                            VkImageAspectFlags aspectFlags) const {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image view!");
        }

        return imageView;
    }

    void DriverVk::createVkTextureSampler(VkSampler &textureSampler) const {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        // The borderColor field specifies which color is returned when sampling beyond the image with clamp to border addressing mode.
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        // All of these fields apply to mipmapping.
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture sampler!");
        }
    }

    void DriverVk::createVkBuffer(VkDeviceSize size,
                                  VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  VkBuffer &buffer,
                                  VkDeviceMemory &bufferMemory) {
        // Structure specifying the parameters of a newly created buffer object.
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size; // Size in bytes of the buffer to be created.
        bufferInfo.usage = usage; // Specifying allowed usages of the buffer.
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Specifying the sharing mode of the buffer when it will be accessed by multiple queue families.

        // Allocate GPU buffer.
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        // Structure containing parameters of a memory allocation.
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                                   properties); // Index identifying a memory type.

        // Allocate CPU buffer memory.
        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        // Bind GPU buffer and CPU buffer memory.
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void DriverVk::createVkRenderPass(VkFormat format, VkRenderPass &renderPass) {
        // Color attachment.
        // ----------------------------------------
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = format; // Specifying the format of the image view that will be used for the attachment.
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Specifying the number of samples of the image.
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Specifying how the contents of color and depth components of the attachment are treated at the beginning of the subpass where it is first used.
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Specifying how the contents of color and depth components of the attachment are treated at the end of the subpass where it is last used.
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // The layout the attachment image subresource will be in when a render pass instance begins.
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // The layout the attachment image subresource will be transitioned to when a render pass instance ends.

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifying the layout the attachment uses during the subpass.
        // ----------------------------------------

        // Depth attachment.
        // ----------------------------------------
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        // ----------------------------------------

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        // Use subpass dependencies for layout transitions.
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

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

        // Create the actual render pass.
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    uint32_t DriverVk::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties;

        // Reports memory information for the specified physical device.
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    VkFormat DriverVk::findSupportedFormat(const std::vector<VkFormat> &candidates,
                                           VkImageTiling tiling,
                                           VkFormatFeatureFlags features) const {
        for (VkFormat format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    VkFormat DriverVk::findDepthFormat() const {
        return findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void DriverVk::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
                                         VkImageLayout oldLayout,
                                         VkImageLayout newLayout) const {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        // Transition barrier masks.
        // -----------------------------
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
//            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//
//            if (hasStencilComponent(format)) {
//                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
        );
        // -----------------------------
    }

    void DriverVk::copyVkBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const {
        // Send copy command.
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    }

    void DriverVk::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image,
                                     uint32_t width,
                                     uint32_t height) const {
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

        region.imageOffset = {0, 0,
                              0}; // Selects the initial x, y, z offsets in texels of the sub-region of the source or destination image data.
        region.imageExtent = {width, height, 1}; // Size in texels of the image to copy in width, height and depth.

        // Copy data from a buffer into an image.
        vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
        );
    }

    void DriverVk::copyDataToMemory(const void *src, VkDeviceMemory bufferMemory, size_t dataSize) const {
        void *data;
        vkMapMemory(device, bufferMemory, 0, dataSize, 0, &data);
        memcpy(data, src, dataSize);
        vkUnmapMemory(device, bufferMemory);
    }
}

#endif
