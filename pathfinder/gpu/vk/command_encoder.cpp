#include "command_encoder.h"

#include <cassert>
#include <functional>

#include "buffer.h"
#include "compute_pipeline.h"
#include "debug_marker.h"
#include "descriptor_set.h"
#include "device.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "texture.h"

namespace Pathfinder {

/// Correct image layout should be set even before binding, not just before submitting command buffer.
/// Also, it can't be set during a render pass.
void transition_image_layout(VkCommandBuffer command_buffer,
                             VkImage image,
                             VkImageLayout old_layout,
                             VkImageLayout new_layout) {
    if (old_layout == new_layout) {
        return;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        //        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        //
        //        if (hasStencilComponent(format)) {
        //            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        //        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    // Undefined -> Transfer dst
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    // Transfer dst -> Sampler
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    // Sampler -> Transfer dst
    else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    // Undefined -> Depth stencil attachment
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    // Image -> Sampler.
    else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    // Sampler -> Image.
    else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    // Undefined -> Sampler
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    // Undefined -> Color attachment
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    // Color attachment -> Sampler
    else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    // Sampler -> Color attachment
    else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    // Transfer dst -> Color attachment
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    // Color attachment -> Transfer src
    else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

CommandEncoderVk::CommandEncoderVk(VkCommandBuffer vk_command_buffer, DeviceVk *device)
    : vk_command_buffer_(vk_command_buffer), vk_device_(device->get_device()), device_vk_(device) {}

CommandEncoderVk::~CommandEncoderVk() {
    invoke_callbacks();

    vkFreeCommandBuffers(vk_device_, device_vk_->get_command_pool(), 1, &vk_command_buffer_);
    vk_command_buffer_ = VK_NULL_HANDLE;
}

VkCommandBuffer CommandEncoderVk::get_vk_handle() const {
    return vk_command_buffer_;
}

bool CommandEncoderVk::finish() {
    if (finished_) {
        Logger::error("Attempted to finished an encoder that's been finished previously!");
        return false;
    }

    // Vulkan will complain if we submit an empty command buffer.
    if (commands_.empty()) {
        return false;
    }

    // Begin recording.
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VK_CHECK_RESULT(vkBeginCommandBuffer(vk_command_buffer_, &begin_info))

    // Start a new debug marker region
    DebugMarker::get_singleton()->begin_region(vk_command_buffer_, label_, ColorF(1.0f, 0.78f, 0.05f, 1.0f));

    for (auto cmd_iter = commands_.begin(); cmd_iter < commands_.end(); cmd_iter++) {
        auto &cmd = *cmd_iter;

        switch (cmd.type) {
            case CommandType::BeginRenderPass: {
                assert(compute_pipeline_ == nullptr);

                bool pass_has_draw_call = false;

                for (auto pass_cmd_iter = cmd_iter; pass_cmd_iter < commands_.end(); pass_cmd_iter++) {
                    if (pass_cmd_iter->type == CommandType::BindDescriptorSet) {
                        sync_descriptor_set(pass_cmd_iter->args.bind_descriptor_set.descriptor_set);
                    }
                    if (pass_cmd_iter->type == CommandType::EndRenderPass) {
                        break;
                    }
                    if (pass_cmd_iter->type == CommandType::Draw || pass_cmd_iter->type == CommandType::DrawInstanced) {
                        pass_has_draw_call = true;
                    }
                }

                auto &args = cmd.args.begin_render_pass;
                auto render_pass_vk = dynamic_cast<RenderPassVk *>(args.render_pass);
                auto framebuffer_vk = dynamic_cast<FramebufferVk *>(args.framebuffer);

                // Transition non-swap-chain-framebuffer image.
                if (framebuffer_vk->get_texture()) {
                    auto texture_vk = dynamic_cast<TextureVk *>(framebuffer_vk->get_texture().get());

                    transition_image_layout(vk_command_buffer_,
                                            texture_vk->get_image(),
                                            to_vk_layout(texture_vk->get_layout()),
                                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

                    texture_vk->set_layout(TextureLayout::ColorAttachment);
                }

                VkRenderPassBeginInfo render_pass_info{};
                render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_info.renderPass = render_pass_vk->get_vk_render_pass();
                render_pass_info.framebuffer = framebuffer_vk->get_vk_handle(); // Set target framebuffer.
                render_pass_info.renderArea.offset = {0, 0};
                // Has to be larger than the area we're going to draw.
                render_pass_info.renderArea.extent =
                    VkExtent2D{uint32_t(framebuffer_vk->get_size().x), uint32_t(framebuffer_vk->get_size().y)};

                // Clear color.
                std::array<VkClearValue, 1> clearValues{};
                clearValues[0].color = {
                    {args.clear_color.r_, args.clear_color.g_, args.clear_color.b_, args.clear_color.a_}};

                render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
                render_pass_info.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(vk_command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

                // In case we need to clear a framebuffer even when nothing is drawn.
                // This is to keep consistency with OpgnGL.
                if (pass_has_draw_call && render_pass_vk->get_attachment_load_op() == AttachmentLoadOp::Clear) {
                    std::array<VkClearAttachment, 1> clear_attachments{};

                    clear_attachments[0] = {VK_IMAGE_ASPECT_COLOR_BIT, 0, clearValues[0]};

                    std::array<VkClearRect, 1> clear_rects{};
                    clear_rects[0] = {
                        VkRect2D{0, 0, (uint32_t)framebuffer_vk->get_size().x, (uint32_t)framebuffer_vk->get_size().y},
                        0,
                        1};

                    vkCmdClearAttachments(vk_command_buffer_,
                                          clear_attachments.size(),
                                          clear_attachments.data(),
                                          clear_rects.size(),
                                          clear_rects.data());
                }
            } break;
            case CommandType::SetViewport: {
                auto &args = cmd.args.set_viewport;

                // Set viewport and scissor dynamically.
                VkViewport viewport{};
                viewport.x = args.viewport.min_x();
                viewport.y = args.viewport.min_y();
                viewport.width = args.viewport.width();
                viewport.height = args.viewport.height();
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(vk_command_buffer_, 0, 1, &viewport);

                VkRect2D scissor{};
                scissor.extent.width = args.viewport.width();
                scissor.extent.height = args.viewport.height();
                vkCmdSetScissor(vk_command_buffer_, 0, 1, &scissor);
            } break;
            case CommandType::BindRenderPipeline: {
                auto &args = cmd.args.bind_render_pipeline;
                auto pipeline_vk = dynamic_cast<RenderPipelineVk *>(args.pipeline);

                render_pipeline_ = pipeline_vk;

                vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_vk->get_pipeline());
            } break;
            case CommandType::BindVertexBuffers: {
                auto &args = cmd.args.bind_vertex_buffers;
                std::vector<VkBuffer> vertex_buffers;
                std::vector<VkDeviceSize> offsets;
                for (uint32_t i = 0; i < args.buffer_count; i++) {
                    auto buffer_vk = dynamic_cast<BufferVk *>(args.buffers[i]);
                    vertex_buffers.push_back(buffer_vk->get_vk_buffer());
                    offsets.push_back(0);
                }

                // Bind vertex and index buffers.
                vkCmdBindVertexBuffers(vk_command_buffer_,
                                       0,
                                       args.buffer_count,
                                       vertex_buffers.data(),
                                       offsets.data()); // Data offset of each buffer.
            } break;
            case CommandType::BindDescriptorSet: {
                auto &args = cmd.args.bind_descriptor_set;
                auto descriptor_set_vk = dynamic_cast<DescriptorSetVk *>(args.descriptor_set);

                if (render_pipeline_) {
                    auto render_pipeline_vk = dynamic_cast<RenderPipelineVk *>(render_pipeline_);

                    descriptor_set_vk->update_vk_descriptor_set(vk_device_,
                                                                render_pipeline_vk->get_descriptor_set_layout());

                    // Bind uniform buffers and samplers.
                    vkCmdBindDescriptorSets(vk_command_buffer_,
                                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            render_pipeline_vk->get_layout(),
                                            0,
                                            1,
                                            &descriptor_set_vk->get_vk_descriptor_set(),
                                            0,
                                            nullptr);
                } else if (compute_pipeline_) {
                    auto compute_pipeline_vk = dynamic_cast<ComputePipelineVk *>(compute_pipeline_);

                    descriptor_set_vk->update_vk_descriptor_set(vk_device_,
                                                                compute_pipeline_vk->get_descriptor_set_layout());

                    // Bind uniform buffers and samplers.
                    vkCmdBindDescriptorSets(vk_command_buffer_,
                                            VK_PIPELINE_BIND_POINT_COMPUTE,
                                            compute_pipeline_vk->get_layout(),
                                            0,
                                            1,
                                            &descriptor_set_vk->get_vk_descriptor_set(),
                                            0,
                                            nullptr);
                } else {
                    Logger::error("No valid pipeline bound when binding descriptor set!", "CommandBuffer");
                    abort();
                }
            } break;
            case CommandType::Draw: {
                auto &args = cmd.args.draw;
                vkCmdDraw(vk_command_buffer_, args.vertex_count, 1, 0, 0);
            } break;
            case CommandType::DrawInstanced: {
                auto &args = cmd.args.draw_instanced;
                vkCmdDraw(vk_command_buffer_, args.vertex_count, args.instance_count, 0, 0);
            } break;
            case CommandType::EndRenderPass: {
                vkCmdEndRenderPass(vk_command_buffer_);

                render_pipeline_ = nullptr;
            } break;
            case CommandType::BeginComputePass: {
                assert(render_pipeline_ == nullptr);

                for (auto cmd_iter2 = cmd_iter; cmd_iter2 < commands_.end(); cmd_iter2++) {
                    if (cmd_iter2->type == CommandType::BindDescriptorSet) {
                        sync_descriptor_set(cmd_iter2->args.bind_descriptor_set.descriptor_set);
                    }
                    if (cmd_iter2->type == CommandType::EndComputePass) {
                        break;
                    }
                }
            } break;
            case CommandType::BindComputePipeline: {
                auto &args = cmd.args.bind_compute_pipeline;

                auto pipeline_vk = dynamic_cast<ComputePipelineVk *>(args.pipeline);

                compute_pipeline_ = pipeline_vk;

                vkCmdBindPipeline(vk_command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_vk->get_pipeline());
            } break;
            case CommandType::Dispatch: {
                auto &args = cmd.args.dispatch;

                // Dispatch compute job.
                vkCmdDispatch(vk_command_buffer_, args.group_size_x, args.group_size_y, args.group_size_z);
            } break;
            case CommandType::EndComputePass: {
                compute_pipeline_ = nullptr;
            } break;
            case CommandType::WriteBuffer: {
                auto &args = cmd.args.write_buffer;
                auto buffer_vk = dynamic_cast<BufferVk *>(args.buffer);

                // Create a host visible buffer and copy data to it by memory mapping.
                // ---------------------------------
                buffer_vk->create_staging_buffer(device_vk_);

                VkBuffer staging_buffer = buffer_vk->vk_staging_buffer_;
                VkDeviceMemory staging_buffer_memory = buffer_vk->vk_staging_device_memory_;

                device_vk_->copy_data_to_mappable_memory(args.data, staging_buffer_memory, args.data_size);
                // ---------------------------------

                device_vk_->copy_vk_buffer(vk_command_buffer_,
                                           staging_buffer,
                                           buffer_vk->get_vk_buffer(),
                                           args.data_size,
                                           0,
                                           args.offset);
            } break;
            // Only for storage buffer.
            case CommandType::ReadBuffer: {
                auto &args = cmd.args.read_buffer;
                auto buffer_vk = dynamic_cast<BufferVk *>(args.buffer);

                // Add a barrier.
                VkBufferMemoryBarrier memory_barrier = {};
                memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                memory_barrier.pNext = nullptr;
                memory_barrier.size = args.data_size;
                memory_barrier.buffer = buffer_vk->get_vk_buffer();
                memory_barrier.offset = args.offset;
                memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                memory_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(vk_command_buffer_,
                                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     0,
                                     0,
                                     nullptr,
                                     1,
                                     &memory_barrier,
                                     0,
                                     nullptr);

                // Create a host visible buffer and copy data to it by memory mapping.
                // ---------------------------------
                buffer_vk->create_staging_buffer(device_vk_);

                VkBuffer staging_buffer = buffer_vk->vk_staging_buffer_;
                VkDeviceMemory staging_buffer_memory = buffer_vk->vk_staging_device_memory_;
                // ---------------------------------

                device_vk_->copy_vk_buffer(vk_command_buffer_,
                                           buffer_vk->get_vk_buffer(),
                                           staging_buffer,
                                           args.data_size);

                auto callback = [this, staging_buffer_memory, args] {
                    // Wait for the data transfer to complete before memory mapping.
                    device_vk_->copy_data_from_mappable_memory(args.data, staging_buffer_memory, args.data_size);
                };
                add_callback(callback);
            } break;
            case CommandType::WriteTexture: {
                auto &args = cmd.args.write_texture;

                auto texture_vk = dynamic_cast<TextureVk *>(args.texture);

                // Image region size in bytes.
                auto pixel_size = get_pixel_size(texture_vk->get_format()); // Bytes of one pixel.

                // Notable that the data size is of the whole texture but of a region.
                VkDeviceSize data_size = args.width * args.height * pixel_size;

                // Prepare staging buffer.
                texture_vk->create_staging_buffer(device_vk_);

                // Copy the pixel data to the staging buffer.
                device_vk_->copy_data_to_mappable_memory(args.data, texture_vk->vk_staging_buffer_memory_, data_size);

                // Transition the image layout to transfer dst.
                transition_image_layout(vk_command_buffer_,
                                        texture_vk->get_image(),
                                        to_vk_layout(texture_vk->get_layout()),
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                texture_vk->set_layout(TextureLayout::TransferDst);

                // Execute the buffer to image copy operation.
                {
                    // Structure specifying a buffer image copy operation.
                    VkBufferImageCopy region{};

                    // Specify which part of the buffer is going to be copied.
                    {
                        region.bufferOffset = 0;      // Byte offset in the buffer at which the pixel values start.
                        region.bufferRowLength = 0;   // Specify how the pixels are laid out in memory.
                        region.bufferImageHeight = 0; // This too.
                    }

                    // Specify which part of the image we want to copy the pixels.
                    {
                        // A VkImageSubresourceLayers used to specify the specific image subresources of the image used
                        // for the source or destination image data.
                        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        region.imageSubresource.mipLevel = 0;
                        region.imageSubresource.baseArrayLayer = 0;
                        region.imageSubresource.layerCount = 1;
                        // Selects the initial x, y, z offsets in texels of the subregion of the source or destination
                        // image data.
                        region.imageOffset = {static_cast<int32_t>(args.offset_x),
                                              static_cast<int32_t>(args.offset_y),
                                              0};
                        // Size in texels of the image to copy in width, height and depth.
                        region.imageExtent = {args.width, args.height, 1};
                    }

                    // Copy data from a buffer into an image.
                    vkCmdCopyBufferToImage(vk_command_buffer_,
                                           texture_vk->vk_staging_buffer_,
                                           texture_vk->get_image(),
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Final image layout after copy.
                                           1,
                                           &region);
                }
            } break;
            case CommandType::ReadTexture: {
                auto &args = cmd.args.read_texture;

                auto texture_vk = dynamic_cast<TextureVk *>(args.texture);

                // Image region size in bytes.
                auto pixel_size = get_pixel_size(texture_vk->get_format()); // Bytes of one pixel.
                VkDeviceSize data_size = args.width * args.height * pixel_size;

                // Prepare staging buffer.
                texture_vk->create_staging_buffer(device_vk_);

                // Transition the image layout to transfer dst.
                transition_image_layout(vk_command_buffer_,
                                        texture_vk->get_image(),
                                        to_vk_layout(texture_vk->get_layout()),
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                texture_vk->set_layout(TextureLayout::TransferSrc);

                // Execute the buffer to image copy operation.
                {
                    // Structure specifying a buffer image copy operation.
                    VkBufferImageCopy region{};

                    // Specify which part of the buffer is going to be copied.
                    {
                        region.bufferOffset = 0;      // Byte offset in the buffer at which the pixel values start.
                        region.bufferRowLength = 0;   // Specify how the pixels are laid out in memory.
                        region.bufferImageHeight = 0; // This too.
                    }

                    // Specify which part of the image we want to copy the pixels.
                    {
                        // A VkImageSubresourceLayers used to specify the specific image subresources of the image used
                        // for the source or destination image data.
                        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        region.imageSubresource.mipLevel = 0;
                        region.imageSubresource.baseArrayLayer = 0;
                        region.imageSubresource.layerCount = 1;
                        // Selects the initial x, y, z offsets in texels of the subregion of the source or destination
                        // image data.
                        region.imageOffset = {static_cast<int32_t>(args.offset_x),
                                              static_cast<int32_t>(args.offset_y),
                                              0};
                        // Size in texels of the image to copy in width, height and depth.
                        region.imageExtent = {args.width, args.height, 1};
                    }

                    // Copy data from a buffer into an image.
                    vkCmdCopyImageToBuffer(vk_command_buffer_,
                                           texture_vk->get_image(),
                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           texture_vk->vk_staging_buffer_,
                                           1,
                                           &region);
                }

                // Callback to clean up staging resources.
                auto callback = [this, texture_vk, data_size, args] {
                    // Copy the pixel data from the staging buffer.
                    device_vk_->copy_data_from_mappable_memory(args.data,
                                                               texture_vk->vk_staging_buffer_memory_,
                                                               data_size);
                };
                add_callback(callback);
            } break;
            case CommandType::Max:
                break;
        }
    }

    commands_.clear();

    DebugMarker::get_singleton()->end_region(vk_command_buffer_);

    // End recording the command buffer.
    VK_CHECK_RESULT(vkEndCommandBuffer(vk_command_buffer_))

    finished_ = true;

    return true;
}

void CommandEncoderVk::sync_descriptor_set(DescriptorSet *descriptor_set) {
    auto descriptor_set_vk = dynamic_cast<DescriptorSetVk *>(descriptor_set);

    // Make all image layouts ready.
    for (auto &d : descriptor_set_vk->get_descriptors()) {
        if (d.second.texture) {
            if (d.second.type == DescriptorType::Sampler) {
                auto texture = d.second.texture.get();
                auto texture_vk = dynamic_cast<TextureVk *>(texture);

                transition_image_layout(vk_command_buffer_,
                                        texture_vk->get_image(),
                                        to_vk_layout(texture_vk->get_layout()),
                                        to_vk_layout(TextureLayout::ShaderReadOnly));

                texture_vk->set_layout(TextureLayout::ShaderReadOnly);
            } else if (d.second.type == DescriptorType::Image) {
                auto texture = d.second.texture.get();
                auto texture_vk = dynamic_cast<TextureVk *>(texture);

                transition_image_layout(vk_command_buffer_,
                                        texture_vk->get_image(),
                                        to_vk_layout(texture_vk->get_layout()),
                                        to_vk_layout(TextureLayout::General));

                texture_vk->set_layout(TextureLayout::General);
            }
        }

        if (d.second.buffer) {
            auto buffer_vk = dynamic_cast<BufferVk *>(d.second.buffer.get());

            int32_t dst_access_mask{};
            switch (buffer_vk->get_type()) {
                case BufferType::Vertex: {
                    Logger::error("Why do we have a vertex buffer in a descriptor set?", "CommandBuffer");
                } break;
                case BufferType::Uniform: {
                    dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
                } break;
                case BufferType::Storage: {
                    dst_access_mask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                } break;
                case BufferType::Index:
                    abort();
            }

            int32_t dst_stage_mask{};
            switch (d.second.stage) {
                case ShaderStage::Vertex: {
                    dst_stage_mask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                } break;
                case ShaderStage::Fragment: {
                    dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                } break;
                case ShaderStage::VertexAndFragment: {
                    dst_stage_mask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                } break;
                case ShaderStage::Compute: {
                    dst_stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                } break;
                default: {
                    abort();
                }
            }

            VkBufferMemoryBarrier memory_barrier = {};
            memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            memory_barrier.pNext = nullptr;
            memory_barrier.size = buffer_vk->get_size();
            memory_barrier.buffer = buffer_vk->get_vk_buffer();
            memory_barrier.offset = 0;
            memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memory_barrier.dstAccessMask = dst_access_mask;

            vkCmdPipelineBarrier(vk_command_buffer_,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 dst_stage_mask,
                                 0,
                                 0,
                                 nullptr,
                                 1,
                                 &memory_barrier,
                                 0,
                                 nullptr);
        }
    }
}

} // namespace Pathfinder
