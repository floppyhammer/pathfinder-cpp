#include "command_buffer.h"
#include "../common/logger.h"

namespace Pathfinder {
    void CommandBuffer::begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                                          const std::shared_ptr<Framebuffer> &framebuffer,
                                          ColorF clear_color) {
        Command cmd;
        cmd.type = CommandType::BeginRenderPass;

        auto &args = cmd.args.begin_render_pass;
        args.render_pass = render_pass.get();
        args.framebuffer = framebuffer.get();
        args.clear_color = clear_color;
        args.extent = framebuffer->get_size();

        commands.push(cmd);
    }

    void CommandBuffer::bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline) {
        Command cmd;
        cmd.type = CommandType::BindRenderPipeline;

        auto &args = cmd.args.bind_render_pipeline;
        args.pipeline = pipeline.get();

        commands.push(cmd);
    }

    void CommandBuffer::bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers) {
        Command cmd;
        cmd.type = CommandType::BindVertexBuffers;

        auto &args = cmd.args.bind_vertex_buffers;
        args.buffer_count = vertex_buffers.size();

        assert(vertex_buffers.size() < MAX_VERTEX_BUFFER_BINDINGS && "Too many vertex buffers bound!");
        for (int buffer_index = 0; buffer_index < vertex_buffers.size(); buffer_index++) {
            args.buffers[buffer_index] = vertex_buffers[buffer_index].get();
        }

        commands.push(cmd);
    }

    void CommandBuffer::bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set) {
        Command cmd;
        cmd.type = CommandType::BindDescriptorSet;

        auto &args = cmd.args.bind_descriptor_set;
        args.descriptor_set = descriptor_set.get();

        commands.push(cmd);
    }

    void CommandBuffer::bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline) {
        Command cmd;
        cmd.type = CommandType::BindComputePipeline;

        auto &args = cmd.args.bind_compute_pipeline;
        args.pipeline = pipeline.get();

        commands.push(cmd);
    }

    void CommandBuffer::draw(uint32_t first_vertex, uint32_t vertex_count) {
        Command cmd;
        cmd.type = CommandType::Draw;

        auto &args = cmd.args.draw;
        args.first_vertex = first_vertex;
        args.vertex_count = vertex_count;

        commands.push(cmd);
    }

    void CommandBuffer::draw_instanced(uint32_t vertex_count, uint32_t instance_count) {
        Command cmd;
        cmd.type = CommandType::DrawInstanced;

        auto &args = cmd.args.draw_instanced;
        args.vertex_count = vertex_count;
        args.instance_count = instance_count;

        commands.push(cmd);
    }

    void CommandBuffer::end_render_pass() {
        Command cmd;
        cmd.type = CommandType::EndRenderPass;

        commands.push(cmd);
    }

    void CommandBuffer::begin_compute_pass() {
        Command cmd;
        cmd.type = CommandType::BeginComputePass;

        commands.push(cmd);
    }

    void CommandBuffer::dispatch(uint32_t group_size_x,
                                 uint32_t group_size_y,
                                 uint32_t group_size_z) {
        if (group_size_x == 0 || group_size_y == 0 || group_size_z == 0) {
            Logger::error("Compute group size cannot be zero!", "CommandBufferVk");
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

    void CommandBuffer::end_compute_pass() {
        Command cmd;
        cmd.type = CommandType::EndComputePass;

        commands.push(cmd);
    }

    void CommandBuffer::upload_to_texture(const std::shared_ptr<Texture> &texture,
                                          Rect<uint32_t> p_region,
                                          const void *data) {
        // Invalid region represents the whole texture.
        auto region = p_region.is_valid() ?
                      p_region : Rect<uint32_t>(0, 0, texture->get_width(),
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

    void CommandBuffer::read_buffer(const std::shared_ptr<Buffer> &buffer,
                                    uint32_t offset,
                                    uint32_t data_size,
                                    void *data) {
        switch (buffer->type) {
            case BufferType::Vertex:
            case BufferType::Index:
            case BufferType::Uniform: {
                Logger::error("It's not possible to read data from non-storage buffers!", "Command Buffer");
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
}