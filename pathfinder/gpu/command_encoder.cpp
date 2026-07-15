#include "command_encoder.h"

#include <cstring>

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

    Command cmd{};
    cmd.type = CommandType::BeginRenderPass;

    auto &args = cmd.args.begin_render_pass;
    args.render_pass = render_pass.get();
    args.framebuffer = framebuffer.get();
    args.clear_color = clear_color;

    commands_.push_back(cmd);
}

void CommandEncoder::set_viewport(const RectI &viewport) {
    Command cmd{};
    cmd.type = CommandType::SetViewport;

    auto &args = cmd.args.set_viewport;
    args.viewport = viewport;

    commands_.push_back(cmd);
}

void CommandEncoder::bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline) {
    assert(pipeline != nullptr);

    Command cmd{};
    cmd.type = CommandType::BindRenderPipeline;

    auto &args = cmd.args.bind_render_pipeline;
    args.pipeline = pipeline.get();

    commands_.push_back(cmd);
}

void CommandEncoder::bind_vertex_buffers(std::vector<std::pair<std::shared_ptr<Buffer>, uint64_t>> vertex_buffers) {
    Command cmd{};
    cmd.type = CommandType::BindVertexBuffers;

    auto &args = cmd.args.bind_vertex_buffers;
    args.buffer_count = vertex_buffers.size();

    assert(vertex_buffers.size() < MAX_VERTEX_BUFFER_BINDINGS && "Too many vertex buffers bound!");
    for (int buffer_index = 0; buffer_index < vertex_buffers.size(); buffer_index++) {
        args.buffers[buffer_index] = vertex_buffers[buffer_index].first.get();
        args.offsets[buffer_index] = vertex_buffers[buffer_index].second;
    }

    commands_.push_back(cmd);
}

void CommandEncoder::bind_index_buffer(Buffer *buffer) {
    Command cmd{};
    cmd.type = CommandType::BindIndexBuffer;

    auto &args = cmd.args.bind_index_buffer;
    args.buffer = buffer;
    args.offset = 0;
    args.data_type = DataType::u32;

    commands_.push_back(cmd);
}

void CommandEncoder::bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set) {
    Command cmd{};
    cmd.type = CommandType::BindDescriptorSet;

    auto &args = cmd.args.bind_descriptor_set;
    args.descriptor_set = descriptor_set.get();

    commands_.push_back(cmd);
}

void CommandEncoder::bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline) {
    Command cmd{};
    cmd.type = CommandType::BindComputePipeline;

    auto &args = cmd.args.bind_compute_pipeline;
    args.pipeline = pipeline.get();

    commands_.push_back(cmd);
}

void CommandEncoder::draw(uint32_t first_vertex, uint32_t vertex_count) {
    Command cmd{};
    cmd.type = CommandType::Draw;

    auto &args = cmd.args.draw;
    args.first_vertex = first_vertex;
    args.vertex_count = vertex_count;

    commands_.push_back(cmd);
}

void CommandEncoder::draw_indexed(uint32_t index_count,
                                  uint32_t instance_count,
                                  uint32_t first_index,
                                  uint32_t first_instance) {
    Command cmd{};
    cmd.type = CommandType::DrawIndexed;

    auto &args = cmd.args.draw_indexed;
    args.index_count = index_count;
    args.first_index = first_index;
    args.instance_count = instance_count;
    args.first_instance = first_instance;

    commands_.push_back(cmd);
}

void CommandEncoder::draw_instanced(uint32_t vertex_count, uint32_t instance_count) {
    Command cmd{};
    cmd.type = CommandType::DrawInstanced;

    auto &args = cmd.args.draw_instanced;
    args.vertex_count = vertex_count;
    args.instance_count = instance_count;

    commands_.push_back(cmd);
}

void CommandEncoder::draw_indirect(Buffer *buffer, uint32_t offset) {
    Command cmd{};
    cmd.type = CommandType::DrawIndirect;

    auto &args = cmd.args.indirect;
    args.buffer = buffer;
    args.offset = offset;

    commands_.push_back(cmd);
}

void CommandEncoder::end_render_pass() {
    Command cmd{};
    cmd.type = CommandType::EndRenderPass;

    commands_.push_back(cmd);
}

void CommandEncoder::begin_compute_pass() {
    Command cmd{};
    cmd.type = CommandType::BeginComputePass;

    commands_.push_back(cmd);
}

void CommandEncoder::dispatch(uint32_t group_size_x, uint32_t group_size_y, uint32_t group_size_z) {
    if (group_size_x == 0 || group_size_y == 0 || group_size_z == 0) {
        Logger::error("Compute group size cannot be zero!");
        return;
    }

    Command cmd{};
    cmd.type = CommandType::Dispatch;

    auto &args = cmd.args.dispatch;
    args.group_size_x = group_size_x;
    args.group_size_y = group_size_y;
    args.group_size_z = group_size_z;

    commands_.push_back(cmd);
}

void CommandEncoder::dispatch_indirect(Buffer *buffer, uint32_t offset) {
    Command cmd{};
    cmd.type = CommandType::DispatchIndirect;

    auto &args = cmd.args.indirect;
    args.buffer = buffer;
    args.offset = offset;

    commands_.push_back(cmd);
}

void CommandEncoder::end_compute_pass() {
    Command cmd{};
    cmd.type = CommandType::EndComputePass;

    commands_.push_back(cmd);
}

void CommandEncoder::write_buffer(const std::shared_ptr<Buffer> &buffer,
                                  uint32_t offset,
                                  uint32_t data_size,
                                  const void *data) {
    if (data_size == 0 || data == nullptr) {
        Logger::error("Tried to write buffer with invalid data!");
        return;
    }

    if (buffer == nullptr) {
        Logger::error("Tried to write invalid buffer!");
        return;
    }

    auto device = device_.lock();
    auto allocation = device->allocate_staging(data_size);

    void *mapped_ptr = device->map_staging(allocation);
    memcpy(mapped_ptr, data, data_size);
    device->unmap_staging(allocation);

    Command cmd{};
    cmd.type = CommandType::WriteBuffer;

    auto &args = cmd.args.write_buffer;
    args.buffer = buffer.get();
    args.offset = offset;
    args.data_size = data_size;
    args.staging_buffer = allocation.buffer.get();
    args.staging_offset = (uint32_t)allocation.offset;

    commands_.push_back(cmd);

    track_temporary_resource(allocation.buffer);
}

void CommandEncoder::read_buffer(const std::shared_ptr<Buffer> &buffer,
                                 uint32_t offset,
                                 uint32_t data_size,
                                 void *data) {
    if (buffer->get_type() != BufferType::Storage) {
        Logger::error("Cannot read data from non-storage buffers!");
        return;
    }

    if (data_size == 0 || data == nullptr) {
        Logger::error("Tried to read invalid data from buffer!");
        return;
    }

    auto device = device_.lock();
    auto allocation = device->allocate_staging(data_size);

    Command cmd{};
    cmd.type = CommandType::WriteBuffer;

    auto &args = cmd.args.read_buffer;
    args.buffer = buffer.get();
    args.offset = offset;
    args.data_size = data_size;
    args.data = data;
    args.staging_buffer = allocation.buffer.get();
    args.staging_offset = (uint32_t)allocation.offset;

    commands_.push_back(cmd);

    add_callback([allocation, data, data_size, device] {
        void *mapped_ptr = device->map_staging(allocation);
        memcpy(data, mapped_ptr, data_size);
        device->unmap_staging(allocation);
    });

    track_temporary_resource(allocation.buffer);
}

void CommandEncoder::write_texture(const std::shared_ptr<Texture> &texture, RectI region, const void *src) {
    auto whole_region = RectI({0, 0}, {texture->get_size()});

    auto effective_region = region.is_valid() ? region : whole_region;

    if (!effective_region.union_rect(whole_region).is_valid() || effective_region.area() == 0) {
        Logger::error("Tried to write invalid region of a texture!");
        return;
    }

    auto pixel_size = get_pixel_size(texture->get_format());
    size_t data_size = (size_t)effective_region.width() * effective_region.height() * pixel_size;

    auto device = device_.lock();
    auto allocation = device->allocate_staging(data_size);

    void *mapped_ptr = device->map_staging(allocation);
    memcpy(mapped_ptr, src, data_size);
    device->unmap_staging(allocation);

    Command cmd{};
    cmd.type = CommandType::WriteTexture;

    auto &args = cmd.args.write_texture;
    args.texture = texture.get();
    args.offset_x = effective_region.left;
    args.offset_y = effective_region.top;
    args.width = effective_region.width();
    args.height = effective_region.height();
    args.staging_buffer = allocation.buffer.get();
    args.staging_offset = (uint32_t)allocation.offset;

    commands_.push_back(cmd);

    track_temporary_resource(allocation.buffer);
}

void CommandEncoder::read_texture(const std::shared_ptr<Texture> &texture, RectI region, void *dst) {
    auto whole_region = RectI({0, 0}, {texture->get_size()});

    auto effective_region = region.is_valid() ? region : whole_region;

    if (!effective_region.union_rect(whole_region).is_valid() || effective_region.area() == 0) {
        Logger::error("Tried to write invalid region of a texture!");
        return;
    }

    auto pixel_size = get_pixel_size(texture->get_format());
    size_t data_size = (size_t)effective_region.width() * effective_region.height() * pixel_size;

    auto device = device_.lock();
    auto allocation = device->allocate_staging(data_size);

    Command cmd{};
    cmd.type = CommandType::ReadTexture;

    auto &args = cmd.args.read_texture;
    args.texture = texture.get();
    args.offset_x = effective_region.left;
    args.offset_y = effective_region.top;
    args.width = effective_region.width();
    args.height = effective_region.height();
    args.data = dst;
    args.staging_buffer = allocation.buffer.get();
    args.staging_offset = (uint32_t)allocation.offset;

    commands_.push_back(cmd);

    add_callback([allocation, dst, data_size, device] {
        void *mapped_ptr = device->map_staging(allocation);
        memcpy(dst, mapped_ptr, data_size);
        device->unmap_staging(allocation);
    });

    track_temporary_resource(allocation.buffer);
}

void CommandEncoder::invoke_callbacks() {
    for (auto &callback : callbacks_) {
        callback();
    }

    callbacks_.clear();
    temp_buffers_.clear();
}

} // namespace Pathfinder
