#include "command_encoder.h"

#include "../common/logger.h"
#include "device.h"

namespace Pathfinder {

void CommandEncoder::begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                                       const std::shared_ptr<Texture> &texture,
                                       ColorF clear_color) {
    std::shared_ptr<Framebuffer> framebuffer;
    if (texture) {
        framebuffer = device_.lock()->create_framebuffer(render_pass, texture, texture->get_label() + " framebuffer");
    } else {
        framebuffer = device_.lock()->create_framebuffer(render_pass, nullptr, "screen framebuffer");
    }
    framebuffers_.push_back(framebuffer);

    Command cmd;
    cmd.type = CommandType::BeginRenderPass;

    auto &args = cmd.args.begin_render_pass;
    args.render_pass = render_pass.get();
    args.framebuffer = framebuffer.get();
    args.clear_color = clear_color;

    commands_.push_back(cmd);
}

void CommandEncoder::set_viewport(const RectI &viewport) {
    Command cmd;
    cmd.type = CommandType::SetViewport;

    auto &args = cmd.args.set_viewport;
    args.viewport = viewport;

    commands_.push_back(cmd);
}

void CommandEncoder::bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline) {
    Command cmd;
    cmd.type = CommandType::BindRenderPipeline;

    auto &args = cmd.args.bind_render_pipeline;
    args.pipeline = pipeline.get();

    commands_.push_back(cmd);
}

void CommandEncoder::bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers) {
    Command cmd;
    cmd.type = CommandType::BindVertexBuffers;

    auto &args = cmd.args.bind_vertex_buffers;
    args.buffer_count = vertex_buffers.size();

    assert(vertex_buffers.size() < MAX_VERTEX_BUFFER_BINDINGS && "Too many vertex buffers bound!");
    for (int buffer_index = 0; buffer_index < vertex_buffers.size(); buffer_index++) {
        args.buffers[buffer_index] = vertex_buffers[buffer_index].get();
    }

    commands_.push_back(cmd);
}

void CommandEncoder::bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set) {
    Command cmd;
    cmd.type = CommandType::BindDescriptorSet;

    auto &args = cmd.args.bind_descriptor_set;
    args.descriptor_set = descriptor_set.get();

    commands_.push_back(cmd);
}

void CommandEncoder::bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline) {
    Command cmd;
    cmd.type = CommandType::BindComputePipeline;

    auto &args = cmd.args.bind_compute_pipeline;
    args.pipeline = pipeline.get();

    commands_.push_back(cmd);
}

void CommandEncoder::draw(uint32_t first_vertex, uint32_t vertex_count) {
    Command cmd;
    cmd.type = CommandType::Draw;

    auto &args = cmd.args.draw;
    args.first_vertex = first_vertex;
    args.vertex_count = vertex_count;

    commands_.push_back(cmd);
}

void CommandEncoder::draw_instanced(uint32_t vertex_count, uint32_t instance_count) {
    Command cmd;
    cmd.type = CommandType::DrawInstanced;

    auto &args = cmd.args.draw_instanced;
    args.vertex_count = vertex_count;
    args.instance_count = instance_count;

    commands_.push_back(cmd);
}

void CommandEncoder::end_render_pass() {
    Command cmd;
    cmd.type = CommandType::EndRenderPass;

    commands_.push_back(cmd);
}

void CommandEncoder::begin_compute_pass() {
    Command cmd;
    cmd.type = CommandType::BeginComputePass;

    commands_.push_back(cmd);
}

void CommandEncoder::dispatch(uint32_t group_size_x, uint32_t group_size_y, uint32_t group_size_z) {
    if (group_size_x == 0 || group_size_y == 0 || group_size_z == 0) {
        Logger::error("Compute group size cannot be zero!", "CommandEncoder");
        return;
    }

    Command cmd;
    cmd.type = CommandType::Dispatch;

    auto &args = cmd.args.dispatch;
    args.group_size_x = group_size_x;
    args.group_size_y = group_size_y;
    args.group_size_z = group_size_z;

    commands_.push_back(cmd);
}

void CommandEncoder::end_compute_pass() {
    Command cmd;
    cmd.type = CommandType::EndComputePass;

    commands_.push_back(cmd);
}

void CommandEncoder::write_buffer(const std::shared_ptr<Buffer> &buffer,
                                  uint32_t offset,
                                  uint32_t data_size,
                                  const void *data) {
    if (data_size == 0 || data == nullptr) {
        Logger::error("Tried to write buffer with invalid data!", "CommandEncoder");
        return;
    }

    if (buffer == nullptr) {
        Logger::error("Tried to write invalid buffer!", "CommandEncoder");
        return;
    }

    // Write buffer by memory mapping.
    if (buffer->get_memory_property() == MemoryProperty::HostVisibleAndCoherent) {
        buffer->upload_via_mapping(data_size, offset, data);
        return;
    }

    Command cmd;
    cmd.type = CommandType::WriteBuffer;

    auto &args = cmd.args.write_buffer;
    args.buffer = buffer.get();
    args.offset = offset;
    args.data_size = data_size;
    args.data = data;

    commands_.push_back(cmd);
}

void CommandEncoder::read_buffer(const std::shared_ptr<Buffer> &buffer,
                                 uint32_t offset,
                                 uint32_t data_size,
                                 void *data) {
    switch (buffer->get_type()) {
        case BufferType::Storage: {
            if (data_size == 0 || data == nullptr) {
                Logger::error("Tried to download invalid data from buffer!");
            }

            // Read buffer by memory mapping.
            if (buffer->get_memory_property() == MemoryProperty::HostVisibleAndCoherent) {
                Logger::error("You're trying to read a mappable buffer through a command. It may indicate some bug.");
                buffer->download_via_mapping(data_size, offset, data);
                return;
            }

            Command cmd;
            cmd.type = CommandType::ReadBuffer;

            auto &args = cmd.args.read_buffer;
            args.buffer = buffer.get();
            args.offset = offset;
            args.data_size = data_size;
            args.data = data;

            commands_.push_back(cmd);
        } break;
        default: {
            Logger::error("Cannot read data from non-storage buffers!", "CommandEncoder");
        } break;
    }
}

void CommandEncoder::write_texture(const std::shared_ptr<Texture> &texture, RectI region, const void *src) {
    auto whole_region = RectI({0, 0}, {texture->get_size()});

    // Invalid region represents the whole texture.
    auto effective_region = region.is_valid() ? region : whole_region;

    // Check if the region is a subset of the whole texture region.
    if (!effective_region.union_rect(whole_region).is_valid() || effective_region.area() == 0) {
        Logger::error("Tried to write invalid region of a texture!", "CommandEncoder");
        return;
    }

    Command cmd;
    cmd.type = CommandType::WriteTexture;

    auto &args = cmd.args.write_texture;
    args.texture = texture.get();
    args.offset_x = effective_region.left;
    args.offset_y = effective_region.top;
    args.width = effective_region.width();
    args.height = effective_region.height();
    args.data = src;

    commands_.push_back(cmd);
}

void CommandEncoder::read_texture(const std::shared_ptr<Texture> &texture, RectI region, void *dst) {
    auto whole_region = RectI({0, 0}, {texture->get_size()});

    // Invalid region represents the whole texture.
    auto effective_region = region.is_valid() ? region : whole_region;

    // Check if the region is a subset of the whole texture region.
    if (!effective_region.union_rect(whole_region).is_valid() || effective_region.area() == 0) {
        Logger::error("Tried to write invalid region of a texture!", "CommandEncoder");
        return;
    }

    Command cmd;
    cmd.type = CommandType::ReadTexture;

    auto &args = cmd.args.read_texture;
    args.texture = texture.get();
    args.offset_x = effective_region.left;
    args.offset_y = effective_region.top;
    args.width = effective_region.width();
    args.height = effective_region.height();
    args.data = dst;

    commands_.push_back(cmd);
}

} // namespace Pathfinder
