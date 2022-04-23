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

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    void CommandBufferVk::begin_render_pass(const std::shared_ptr<Framebuffer> &framebuffer,
                                            bool clear,
                                            ColorF clear_color) {
        Command cmd;
        cmd.type = CommandType::BeginRenderPass;

        auto &args = cmd.args.begin_render_pass;
        args.framebuffer = framebuffer.get();
        args.extent = {framebuffer->get_width(), framebuffer->get_height()};
        args.clear = clear;
        args.clear_color = clear_color;

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
        if (vkBeginCommandBuffer(vk_command_buffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        while (!commands.empty()) {
            auto &cmd = commands.front();

            switch (cmd.type) {
                case CommandType::BeginRenderPass: {
                    auto &args = cmd.args.begin_render_pass;
                    auto render_pass_vk = static_cast<RenderPassVk *>(args.render_pass);
                    auto framebuffer_vk = static_cast<FramebufferVk *>(args.framebuffer);

                    VkRenderPassBeginInfo renderPassInfo{};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    renderPassInfo.renderPass = render_pass_vk->get_render_pass();
                    renderPassInfo.framebuffer = framebuffer_vk->get_framebuffer_id(); // Set target framebuffer.
                    renderPassInfo.renderArea.offset = {0, 0};
                    renderPassInfo.renderArea.extent = VkExtent2D{args.extent.x, args.extent.y}; // Has to be larger than the area we're going to draw.

                    // Clear color.
                    std::array<VkClearValue, 1> clearValues{};
                    clearValues[0].color = {{0.02f, 0.02f, 0.02f, 1.0f}};

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

                    vkCmdBindPipeline(vk_command_buffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      pipeline_vk->get_pipeline());
                }
                    break;
                case CommandType::BindVertexBuffers: {
                    auto &args = cmd.args.bind_vertex_buffers;
                    std::vector<VkBuffer> vertex_buffers(args.buffer_count);
                    for (uint32_t i = 0; i < args.buffer_count; i++) {
                        auto buffer_vk = static_cast<BufferVk *>(args.buffers[i]);
                        vertex_buffers.push_back(buffer_vk->get_vk_buffer());
                    }

                    // Bind vertex and index buffers.
                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(vk_command_buffer,
                                           0,
                                           1,
                                           vertex_buffers.data(),
                                           offsets);
                }
                    break;
                case CommandType::BindDescriptorSet: {
                    auto &args = cmd.args.bind_descriptor_set;
                    auto descriptor_set_vk = static_cast<DescriptorSetVk *>(args.descriptor_set);
                    auto render_pipeline_vk = static_cast<RenderPipelineVk *>(render_pipeline);

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
                }
                    break;
                case CommandType::UploadToBuffer: {
                    auto &args = cmd.args.upload_to_buffer;

                }
                    break;
                case CommandType::ReadBuffer: {
                    auto &args = cmd.args.read_buffer;

                }
                    break;
                case CommandType::UploadToTexture: {
                    auto &args = cmd.args.upload_to_texture;

                    auto texture_vk = static_cast<TextureVk *>(args.texture);

                    // In bytes. 4 bytes per pixel.
                    VkDeviceSize imageSize = args.width * args.height * 4;

                    // Temporary buffer and CPU memory.
                    VkBuffer stagingBuffer;
                    VkDeviceMemory stagingBufferMemory;

                    driver->createVkBuffer(imageSize,
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           stagingBuffer,
                                           stagingBufferMemory);

                    // Copy the pixel values that we got from the image loading library to the buffer.
                    driver->copyDataToMemory(args.data, stagingBufferMemory, imageSize);

                    // Transition the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
                    driver->transitionImageLayout(vk_command_buffer,
                                                  texture_vk->get_image(),
                                                  VK_FORMAT_R8G8B8A8_SRGB,
                                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                    // Execute the buffer to image copy operation.
                    {
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
                        // Selects the initial x, y, z offsets in texels of the sub-region of the source or destination image data.
                        region.imageOffset = {0, 0, 0};
                        // Size in texels of the image to copy in width, height and depth.
                        region.imageExtent = {texture_vk->get_width(), texture_vk->get_height(), 1};

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
                                                  VK_FORMAT_R8G8B8A8_SRGB,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                    // Clean up staging stuff.
                    vkDestroyBuffer(driver->get_device(), stagingBuffer, nullptr);
                    vkFreeMemory(driver->get_device(), stagingBufferMemory, nullptr);
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

        // Submit the command buffer to the graphics queue.
        // ----------------------------------------
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vk_command_buffer;

        vkQueueSubmit(driver->get_queue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(driver->get_queue());
        // ----------------------------------------

        // Free the command buffer.
        //vkFreeCommandBuffers(driver->get_device(), commandPool, 1, &vk_command_buffer);
    }
}

#endif