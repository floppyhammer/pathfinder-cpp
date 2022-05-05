#include "command_buffer.h"

#include "buffer.h"
#include "driver.h"
#include "texture.h"
#include "render_pass.h"
#include "framebuffer.h"
#include "render_pipeline.h"
#include "descriptor_set.h"
#include "../../common/logger.h"

#include <cassert>
#include <functional>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    CommandBufferVk::CommandBufferVk(VkCommandBuffer command_buffer, VkDevice device)
            : vk_command_buffer(command_buffer), vk_device(device) {}

    void CommandBufferVk::begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                                            const std::shared_ptr<Framebuffer> &framebuffer,
                                            bool clear,
                                            ColorF clear_color) {
        Command cmd;
        cmd.type = CommandType::BeginRenderPass;

        auto &args = cmd.args.begin_render_pass;
        args.render_pass = render_pass.get();
        args.framebuffer = framebuffer.get();
        args.clear = clear;
        args.clear_color = clear_color;
        args.extent = framebuffer->get_size();

        commands.push(cmd);
    }

    void CommandBuffer::end_render_pass() {

    }

    void CommandBufferVk::bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline) {
        Command cmd;
        cmd.type = CommandType::BindRenderPipeline;
        auto &args = cmd.args.bind_render_pipeline;
        args.pipeline = pipeline.get();

        commands.push(cmd);
    }

    void CommandBufferVk::bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline) {
        Command cmd;
        cmd.type = CommandType::BindComputePipeline;
        auto &args = cmd.args.bind_compute_pipeline;
        args.pipeline = pipeline.get();

        commands.push(cmd);
    }

    void CommandBufferVk::bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers) {
        Command cmd;
        cmd.type = CommandType::BindVertexBuffers;
        auto &args = cmd.args.bind_vertex_buffers;
        args.buffer_count = vertex_buffers.size();

        assert(vertex_buffers.size() < 10 && "Maximum vertex buffer per pipeline is 10!");
        for (int buffer_index = 0; buffer_index < vertex_buffers.size(); buffer_index++) {
            args.buffers[buffer_index] = vertex_buffers[buffer_index].get();
        }

        commands.push(cmd);
    }

    void CommandBufferVk::bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set) {
        Command cmd;
        cmd.type = CommandType::BindDescriptorSet;

        auto &args = cmd.args.bind_descriptor_set;
        args.descriptor_set = descriptor_set.get();

        commands.push(cmd);
    }

    void CommandBufferVk::draw(uint32_t first_vertex, uint32_t vertex_count) {
        Command cmd;
        cmd.type = CommandType::Draw;

        auto &args = cmd.args.draw;
        args.first_vertex = first_vertex;
        args.vertex_count = vertex_count;

        commands.push(cmd);
    }

    void CommandBufferVk::draw_instanced(uint32_t vertex_count, uint32_t instance_count) {
        Command cmd;
        cmd.type = CommandType::DrawInstanced;

        auto &args = cmd.args.draw_instanced;
        args.vertex_count = vertex_count;
        args.instance_count = instance_count;

        commands.push(cmd);
    }

    void CommandBufferVk::end_render_pass() {
        Command cmd;
        cmd.type = CommandType::EndRenderPass;

        commands.push(cmd);
    }

    void CommandBufferVk::begin_compute_pass() {

    }

    void CommandBufferVk::dispatch(uint32_t group_size_x,
                                   uint32_t group_size_y,
                                   uint32_t group_size_z) {
        if (group_size_x == 0 || group_size_y == 0 || group_size_z == 0) {
            Logger::error("Compute group size cannot be zero!", "ComputeProgram");
            return;
        }

        Command cmd;
        cmd.type = CommandType::Dispatch;

        auto &args = cmd.args.dispatch;
        args.group_size_x = group_size_x;
        args.group_size_y = group_size_y;
        args.group_size_z = group_size_z;

        commands.push(cmd);
    }

    void CommandBufferVk::end_compute_pass() {

    }

    void CommandBufferVk::upload_to_buffer(const std::shared_ptr<Buffer> &buffer, uint32_t offset, uint32_t data_size,
                                           void *data) {
        if (data_size == 0 || data == nullptr) {
            Logger::error("Tried to upload invalid data to buffer!");
        }

        // Update buffer by memory mapping.
        if (buffer->get_memory_property() == MemoryProperty::HOST_VISIBLE_AND_COHERENT) {
            auto buffer_vk = static_cast<BufferVk *>(buffer.get());

            void *mapped_data;
            auto res = vkMapMemory(vk_device, buffer_vk->get_vk_device_memory(), offset, data_size, 0, &mapped_data);
            memcpy(mapped_data, data, data_size);
            vkUnmapMemory(vk_device, buffer_vk->get_vk_device_memory());

            if (res != VK_SUCCESS) {
                Logger::error("Failed to map memory!", "Vulkan");
            }

            return;
        }

        Command cmd;
        cmd.type = CommandType::UploadToBuffer;

        auto &args = cmd.args.upload_to_buffer;
        args.buffer = buffer.get();
        args.offset = offset;
        args.data_size = data_size;
        args.data = data;

        commands.push(cmd);
    }

    void CommandBufferVk::upload_to_texture(const std::shared_ptr<Texture> &texture, Rect<uint32_t> p_region,
                                            const void *data) {
        // Invalid region represents the whole texture.
        auto region = p_region.is_valid() ? p_region : Rect<uint32_t>(0, 0, texture->get_width(),
                                                                      texture->get_height());

        Command cmd;
        cmd.type = CommandType::UploadToTexture;

        auto &args = cmd.args.upload_to_texture;
        args.texture = texture.get();
        args.offset_x = region.left;
        args.offset_y = region.top;
        args.width = region.width();
        args.height = region.height();
        args.data = data;

        commands.push(cmd);
    }

    void CommandBufferVk::read_buffer(const std::shared_ptr<Buffer> &buffer,
                                      uint32_t offset,
                                      uint32_t data_size,
                                      void *data) {
        switch (buffer->type) {
            case BufferType::Vertex:
            case BufferType::Uniform: {
                Logger::error("It's not possible to read data from vertex/uniform buffers!", "Command Buffer");
            }
                break;
            case BufferType::General: {
                Command cmd;
                cmd.type = CommandType::ReadBuffer;

                auto &args = cmd.args.read_buffer;
                args.buffer = buffer.get();
                args.offset = offset;
                args.data_size = data_size;
                args.data = data;

                commands.push(cmd);
            }
                break;
        }
    }

    void CommandBufferVk::submit(const std::shared_ptr<Driver> &p_driver) {
        auto driver = dynamic_cast<DriverVk *>(p_driver.get());

        // Begin recording.
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (one_time) {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        if (vkBeginCommandBuffer(vk_command_buffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        std::vector<std::function<void()>> callbacks;

        while (!commands.empty()) {
            auto &cmd = commands.front();

            switch (cmd.type) {
                case CommandType::BeginRenderPass: {
                    auto &args = cmd.args.begin_render_pass;
                    auto render_pass_vk = static_cast<RenderPassVk *>(args.render_pass);
                    auto framebuffer_vk = static_cast<FramebufferVk *>(args.framebuffer);

                    VkRenderPassBeginInfo renderPassInfo{};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    renderPassInfo.renderPass = render_pass_vk->get_vk_render_pass();
                    renderPassInfo.framebuffer = framebuffer_vk->get_vk_framebuffer(); // Set target framebuffer.
                    renderPassInfo.renderArea.offset = {0, 0};
                    // Has to be larger than the area we're going to draw.
                    renderPassInfo.renderArea.extent = VkExtent2D{args.extent.x, args.extent.y};

                    // Clear color.
                    std::array<VkClearValue, 1> clearValues{};
                    clearValues[0].color = {{
                                                    args.clear_color.r,
                                                    args.clear_color.g,
                                                    args.clear_color.b,
                                                    args.clear_color.a}};

                    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                    renderPassInfo.pClearValues = clearValues.data();

                    vkCmdBeginRenderPass(vk_command_buffer,
                                         &renderPassInfo,
                                         VK_SUBPASS_CONTENTS_INLINE);
                }
                    break;
                case CommandType::BindRenderPipeline: {
                    auto &args = cmd.args.bind_render_pipeline;
                    auto pipeline_vk = static_cast<RenderPipelineVk *>(args.pipeline);

                    render_pipeline = pipeline_vk;

                    vkCmdBindPipeline(vk_command_buffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      pipeline_vk->get_pipeline());
                }
                    break;
                case CommandType::BindVertexBuffers: {
                    auto &args = cmd.args.bind_vertex_buffers;
                    std::vector<VkBuffer> vertex_buffers;
                    std::vector<VkDeviceSize> offsets;
                    for (uint32_t i = 0; i < args.buffer_count; i++) {
                        auto buffer_vk = static_cast<BufferVk *>(args.buffers[i]);
                        vertex_buffers.push_back(buffer_vk->get_vk_buffer());
                        offsets.push_back(0);
                    }

                    // Bind vertex and index buffers.
                    vkCmdBindVertexBuffers(vk_command_buffer,
                                           0,
                                           args.buffer_count,
                                           vertex_buffers.data(),
                                           offsets.data()); // Data offset of each buffer.
                }
                    break;
                case CommandType::BindDescriptorSet: {
                    auto &args = cmd.args.bind_descriptor_set;
                    auto descriptor_set_vk = static_cast<DescriptorSetVk *>(args.descriptor_set);
                    auto render_pipeline_vk = static_cast<RenderPipelineVk *>(render_pipeline);

                    descriptor_set_vk->update_vk_descriptor_set(driver->get_device(),
                                                                render_pipeline_vk->get_descriptor_set_layout());

                    // Bind uniform buffers and samplers.
                    vkCmdBindDescriptorSets(vk_command_buffer,
                                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            render_pipeline_vk->get_layout(),
                                            0,
                                            1,
                                            &descriptor_set_vk->get_vk_descriptor_set(),
                                            0,
                                            nullptr);
                }
                    break;
                case CommandType::Draw: {
                    auto &args = cmd.args.draw;
                    vkCmdDraw(vk_command_buffer, args.vertex_count, 1, 0, 0);
                }
                    break;
                case CommandType::DrawInstanced: {
                    auto &args = cmd.args.draw_instanced;
                    vkCmdDraw(vk_command_buffer, args.vertex_count, args.instance_count, 0, 0);
                }
                    break;
                case CommandType::EndRenderPass: {
                    vkCmdEndRenderPass(vk_command_buffer);

                    render_pipeline = nullptr;
                }
                    break;
                case CommandType::BeginComputePass: {
                }
                    break;
                case CommandType::BindComputePipeline: {
                    auto &args = cmd.args.bind_compute_pipeline;
                }
                    break;
                case CommandType::Dispatch: {
                    auto &args = cmd.args.dispatch;
                }
                    break;
                case CommandType::EndComputePass: {
                    compute_pipeline = nullptr;
                }
                    break;
                case CommandType::UploadToBuffer: {
                    auto &args = cmd.args.upload_to_buffer;
                    auto buffer_vk = static_cast<BufferVk *>(args.buffer);

                    // Create a host visible buffer and copy data to it by memory mapping.
                    // ---------------------------------
                    VkBuffer stagingBuffer;
                    VkDeviceMemory stagingBufferMemory;

                    driver->createVkBuffer(args.data_size,
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           stagingBuffer,
                                           stagingBufferMemory);

                    driver->copyDataToMemory(args.data,
                                             stagingBufferMemory,
                                             args.data_size);
                    // ---------------------------------

                    driver->copyVkBuffer(vk_command_buffer, stagingBuffer, buffer_vk->get_vk_buffer(), args.data_size);

                    // Callback to clean up staging resources.
                    auto callback = [driver, stagingBuffer, stagingBufferMemory] {
                        vkDestroyBuffer(driver->get_device(), stagingBuffer, nullptr);
                        vkFreeMemory(driver->get_device(), stagingBufferMemory, nullptr);
                    };
                    callbacks.push_back(callback);
                }
                    break;
                case CommandType::ReadBuffer: {
                    auto &args = cmd.args.read_buffer;

                }
                    break;
                case CommandType::UploadToTexture: {
                    auto &args = cmd.args.upload_to_texture;

                    auto texture_vk = static_cast<TextureVk *>(args.texture);

                    // Image region size in bytes.
                    auto pixel_size = get_pixel_size(texture_vk->get_format()); // Bytes of one pixel.
                    VkDeviceSize dataSize = args.width * args.height * pixel_size;

                    // Temporary buffer and CPU memory.
                    VkBuffer stagingBuffer;
                    VkDeviceMemory stagingBufferMemory;

                    driver->createVkBuffer(dataSize,
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           stagingBuffer,
                                           stagingBufferMemory);

                    // Copy the pixel values that we got from the image loading library to the buffer.
                    driver->copyDataToMemory(args.data, stagingBufferMemory, dataSize);

                    // Transition the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
                    driver->transitionImageLayout(vk_command_buffer,
                                                  texture_vk->get_image(),
                                                  to_vk_texture_format(texture_vk->get_format()),
                                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                    // Execute the buffer to image copy operation.
                    {
                        // Structure specifying a buffer image copy operation.
                        VkBufferImageCopy region{};

                        // Specify which part of the buffer is going to be copied.
                        {
                            region.bufferOffset = 0; // Byte offset in the buffer at which the pixel values start.
                            region.bufferRowLength = 0; // Specify how the pixels are laid out in memory.
                            region.bufferImageHeight = 0; // This too.
                        }

                        // Specify which part of the image we want to copy the pixels.
                        {
                            // A VkImageSubresourceLayers used to specify the specific image subresources of the image used for the source or destination image data.
                            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                            region.imageSubresource.mipLevel = 0;
                            region.imageSubresource.baseArrayLayer = 0;
                            region.imageSubresource.layerCount = 1;
                            // Selects the initial x, y, z offsets in texels of the sub-region of the source or destination image data.
                            region.imageOffset = {static_cast<int32_t>(args.offset_x),
                                                  static_cast<int32_t>(args.offset_y),
                                                  0};
                            // Size in texels of the image to copy in width, height and depth.
                            region.imageExtent = {args.width, args.height, 1};
                        }

                        // Copy data from a buffer into an image.
                        vkCmdCopyBufferToImage(vk_command_buffer,
                                               stagingBuffer,
                                               texture_vk->get_image(),
                                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                               1,
                                               &region
                        );
                    }

                    // To be able to start sampling from the texture image in the shader, we need one last transition to prepare it for shader access.
                    driver->transitionImageLayout(vk_command_buffer,
                                                  texture_vk->get_image(),
                                                  to_vk_texture_format(texture_vk->get_format()),
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                    // Callback to clean up staging resources.
                    auto callback = [driver, stagingBuffer, stagingBufferMemory] {
                        vkDestroyBuffer(driver->get_device(), stagingBuffer, nullptr);
                        vkFreeMemory(driver->get_device(), stagingBufferMemory, nullptr);
                    };
                    callbacks.push_back(callback);
                }
                    break;
                case CommandType::Max:
                    break;
            }

            commands.pop();
        }

        // End recording the command buffer.
        if (vkEndCommandBuffer(vk_command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }

        if (one_time) {
            // Submit the command buffer to the graphics queue.
            // ----------------------------------------
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &vk_command_buffer;

            vkQueueSubmit(driver->get_graphics_queue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(driver->get_graphics_queue());
            // ----------------------------------------

            // Free the command buffer.
            vkFreeCommandBuffers(driver->get_device(), driver->get_command_pool(), 1, &vk_command_buffer);
        }

        for (auto &callback: callbacks) {
            callback();
        }
    }
}

#endif
