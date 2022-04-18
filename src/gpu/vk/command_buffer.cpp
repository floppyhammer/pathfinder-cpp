#include "command_buffer.h"

#include "buffer.h"

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

    void CommandBufferVk::submit() {
        while (!commands.empty()) {
            auto &cmd = commands.front();

            switch (cmd.type) {
                case CommandType::BeginRenderPass: {

                }
                    break;
                case CommandType::BindRenderPipeline: {

                }
                    break;
                case CommandType::BindVertexBuffers: {

                }
                    break;
                case CommandType::BindDescriptorSet: {
                    auto &args = cmd.args.bind_descriptor_set;

                }
                    break;
                case CommandType::Draw: {
                    auto &args = cmd.args.draw;

                }
                    break;
                case CommandType::DrawInstanced: {
                    auto &args = cmd.args.draw_instanced;

                }
                    break;
                case CommandType::EndRenderPass: {

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

                    // In bytes. 4 bytes per pixel.
                    VkDeviceSize imageSize = tex_width * tex_height * 4;

                    // Temporary buffer and CPU memory.
                    VkBuffer stagingBuffer;
                    VkDeviceMemory stagingBufferMemory;

                    RS::getSingleton().createBuffer(imageSize,
                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                    stagingBuffer,
                                                    stagingBufferMemory);

                    // Copy the pixel values that we got from the image loading library to the buffer.
                    RS::getSingleton().copyDataToMemory(pixels, stagingBufferMemory, imageSize);

                    // Transition the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
                    RS::getSingleton().transitionImageLayout(image,
                                                             VK_FORMAT_R8G8B8A8_SRGB,
                                                             VK_IMAGE_LAYOUT_UNDEFINED,
                                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                    // Execute the buffer to image copy operation.
                    RS::getSingleton().copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(width),
                                                         static_cast<uint32_t>(height));

                    // To be able to start sampling from the texture image in the shader, we need one last transition to prepare it for shader access.
                    RS::getSingleton().transitionImageLayout(image,
                                                             VK_FORMAT_R8G8B8A8_SRGB,
                                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                    // Clean up staging stuff.
                    vkDestroyBuffer(Device::getSingleton().device, stagingBuffer, nullptr);
                    vkFreeMemory(Device::getSingleton().device, stagingBufferMemory, nullptr);
                }
                    break;
                case CommandType::Max:
                    break;
            }

            commands.pop();
        }
    }
}

#endif
