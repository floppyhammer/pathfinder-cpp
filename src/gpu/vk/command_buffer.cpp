#include "command_buffer.h"

#include "buffer.h"
#include "driver.h"
#include "texture.h"
#include "render_pass.h"
#include "framebuffer.h"
#include "render_pipeline.h"
#include "compute_pipeline.h"
#include "descriptor_set.h"
#include "../command_buffer.h"
#include "../../common/logger.h"

#include <cassert>
#include <functional>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    CommandBufferVk::CommandBufferVk(VkCommandBuffer p_command_buffer, VkDevice p_device)
            : vk_command_buffer(p_command_buffer), device(p_device) {}

    void CommandBufferVk::upload_to_buffer(const std::shared_ptr<Buffer> &buffer,
                                           uint32_t offset,
                                           uint32_t data_size,
                                           void *data) {
        if (data_size == 0 || data == nullptr) {
            Logger::error("Tried to upload invalid data to buffer!");
        }

        // Update buffer by memory mapping.
        if (buffer->get_memory_property() == MemoryProperty::HOST_VISIBLE_AND_COHERENT) {
            auto buffer_vk = static_cast<BufferVk *>(buffer.get());

            void *mapped_data;
            auto res = vkMapMemory(device, buffer_vk->get_vk_device_memory(), offset, data_size, 0, &mapped_data);
            memcpy(mapped_data, data, data_size);
            vkUnmapMemory(device, buffer_vk->get_vk_device_memory());

            if (res != VK_SUCCESS) {
                Logger::error("Failed to map memory!", "CommandBufferVk");
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

                    // Set viewport and scissor dynamically.
                    VkViewport viewport{};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = (float) framebuffer_vk->get_width();
                    viewport.height = (float) framebuffer_vk->get_height();
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    vkCmdSetViewport(vk_command_buffer, 0, 1, &viewport);

                    VkRect2D scissor{};
                    scissor.extent.width = framebuffer_vk->get_width();
                    scissor.extent.height = framebuffer_vk->get_height();
                    vkCmdSetScissor(vk_command_buffer, 0, 1, &scissor);
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

                    if (render_pipeline) {
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
                    } else if (compute_pipeline) {
                        auto compute_pipeline_vk = static_cast<ComputePipelineVk *>(compute_pipeline);

                        descriptor_set_vk->update_vk_descriptor_set(driver->get_device(),
                                                                    compute_pipeline_vk->get_descriptor_set_layout());

                        // Bind uniform buffers and samplers.
                        vkCmdBindDescriptorSets(vk_command_buffer,
                                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                                compute_pipeline_vk->get_layout(),
                                                0,
                                                1,
                                                &descriptor_set_vk->get_vk_descriptor_set(),
                                                0,
                                                nullptr);
                    } else {
                        Logger::error("No valid pipeline bound when binding descriptor set!", "Command Buffer");
                        abort();
                    }
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

                    auto pipeline_vk = static_cast<ComputePipelineVk *>(args.pipeline);

                    compute_pipeline = pipeline_vk;

                    vkCmdBindPipeline(vk_command_buffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      pipeline_vk->get_pipeline());
                }
                    break;
                case CommandType::Dispatch: {
                    auto &args = cmd.args.dispatch;

                    // Dispatch compute job.
                    vkCmdDispatch(vk_command_buffer, args.group_size_x, args.group_size_y, args.group_size_z);
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

//                    VkBufferMemoryBarrier barrier;
//                    barrier.pNext = nullptr;
//                    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
//                    barrier.buffer = buffer_vk->get_vk_buffer();
//                    barrier.offset = 0;
//                    barrier.size = buffer_vk->size;
//                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//
//                    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
//                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//
//                    vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
//                                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
//                                         0, nullptr, 1, &barrier, 0, nullptr);

                    driver->copyVkBuffer(vk_command_buffer, stagingBuffer, buffer_vk->get_vk_buffer(), args.data_size);

//                    // Don't read vertex data as we're writing it.
//                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//                    barrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
//
//                    vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
//                                         VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0,
//                                         0, nullptr, 1, &barrier, 0, nullptr);

                    // Callback to clean up staging resources.
                    auto callback = [driver, stagingBuffer, stagingBufferMemory] {
                        vkDestroyBuffer(driver->get_device(), stagingBuffer, nullptr);
                        vkFreeMemory(driver->get_device(), stagingBufferMemory, nullptr);
                    };
                    add_callback(callback);
                }
                    break;
                case CommandType::ReadBuffer: {
                    auto &args = cmd.args.read_buffer;
                    auto buffer_vk = static_cast<BufferVk *>(args.buffer);

                    // Create a host visible buffer and copy data to it by memory mapping.
                    // ---------------------------------
                    VkBuffer staging_buffer;
                    VkDeviceMemory staging_buffer_memory;

                    driver->createVkBuffer(args.data_size,
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           staging_buffer,
                                           staging_buffer_memory);
                    // ---------------------------------

                    driver->copyVkBuffer(vk_command_buffer, buffer_vk->get_vk_buffer(), staging_buffer, args.data_size);

                    // Callback to clean up staging resources.
                    auto callback = [driver, staging_buffer, staging_buffer_memory, args] {
                        // Wait for the data transfer to complete before memory mapping.
                        driver->copyDataFromMemory(args.data,
                                                   staging_buffer_memory,
                                                   args.data_size);

                        vkDestroyBuffer(driver->get_device(), staging_buffer, nullptr);
                        vkFreeMemory(driver->get_device(), staging_buffer_memory, nullptr);
                    };
                    add_callback(callback);
                }
                    break;
                case CommandType::UploadToTexture: {
                    auto &args = cmd.args.upload_to_texture;

                    auto texture_vk = static_cast<TextureVk *>(args.texture);

                    // Image region size in bytes.
                    auto pixel_size = get_pixel_size(texture_vk->get_format()); // Bytes of one pixel.
                    VkDeviceSize dataSize = args.width * args.height * pixel_size;

                    // Temporary buffer and device memory.
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
                    add_callback(callback);
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

        callbacks.clear();
    }
}

#endif
