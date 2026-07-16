#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>

#include "../common/color.h"
#include "../common/math/rect.h"
#include "buffer.h"
#include "compute_pipeline.h"
#include "descriptor_set.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"

namespace Pathfinder {

class Device;

enum class CommandType {
    // RENDER PASS

    BeginRenderPass = 0,
    SetViewport,
    BindRenderPipeline,
    BindVertexBuffers,
    BindIndexBuffer,
    BindDescriptorSet,
    Draw,
    DrawIndexed,
    DrawInstanced,
    DrawIndirect,
    EndRenderPass,

    // COMPUTE PASS

    BeginComputePass,
    BindComputePipeline,
    Dispatch,
    DispatchIndirect,
    EndComputePass,

    // DATA TRANSFER

    WriteBuffer,
    ReadBuffer,
    WriteTexture,
    ReadTexture,

    Max,
};

struct StagingAllocation {
    std::shared_ptr<Buffer> buffer;
    size_t offset = 0;
    size_t data_size = 0;
    void *mapped_ptr = nullptr;
};

struct Command {
    CommandType type = CommandType::Max;

    union Args {
        struct {
            RenderPass *render_pass;
            Framebuffer *framebuffer;
            ColorF clear_color;
        } begin_render_pass{};
        struct {
            RectI viewport;
        } set_viewport;
        struct {
            RenderPipeline *pipeline;
        } bind_render_pipeline;
        struct {
            uint32_t buffer_count;
            std::array<Buffer *, MAX_VERTEX_BUFFER_BINDINGS> buffers;
            std::array<uint64_t, MAX_VERTEX_BUFFER_BINDINGS> offsets;
        } bind_vertex_buffers;
        struct {
            Buffer *buffer;
            DataType data_type; // u32 only.
            size_t offset;      // Zero only.
        } bind_index_buffer;
        struct {
            DescriptorSet *descriptor_set;
        } bind_descriptor_set;
        struct {
            uint32_t first_vertex;
            uint32_t vertex_count;
        } draw;
        struct {
            uint32_t index_count;
            uint32_t first_index;
            uint32_t instance_count;
            uint32_t first_instance;
        } draw_indexed;
        struct {
            uint32_t vertex_count;
            uint32_t instance_count;
        } draw_instanced;
        struct {
            Buffer *buffer;
            size_t offset;
        } indirect;
        struct {
            ComputePipeline *pipeline;
        } bind_compute_pipeline;
        struct {
            uint32_t group_size_x;
            uint32_t group_size_y;
            uint32_t group_size_z;
        } dispatch;
        struct {
            Buffer *buffer;
            uint32_t offset;
            uint32_t data_size;
            const void *data;
            Buffer *staging_buffer;
            uint32_t staging_offset;
        } write_buffer;
        struct {
            Buffer *buffer;
            uint32_t offset;
            uint32_t data_size;
            void *data;
            Buffer *staging_buffer;
            uint32_t staging_offset;
        } read_buffer;
        struct {
            Texture *texture;
            uint32_t offset_x;
            uint32_t offset_y;
            uint32_t width;
            uint32_t height;
            const void *data;
            Buffer *staging_buffer;
            uint32_t staging_offset;
        } write_texture;
        struct {
            Texture *texture;
            uint32_t offset_x;
            uint32_t offset_y;
            uint32_t width;
            uint32_t height;
            void *data;
            Buffer *staging_buffer;
            uint32_t staging_offset;
        } read_texture;
    } args;
};

/// Don't do any modifications to command arguments until the buffer is submitted.
/// Maybe this command buffer will not be executed at all.
/// Or maybe a later generated command buffer will be executed first.
class CommandEncoder {
    friend class QueueVk;
    friend class QueueGl;
    friend class SwapChainGl;
    friend class SwapChainVk;

public:
    virtual ~CommandEncoder() = default;

    // RENDER PASS

    void begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                           const std::shared_ptr<Texture> &texture,
                           ColorF clear_color);

    void set_viewport(const RectI &viewport);

    /// Bind pipeline.
    void bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline);

    void bind_vertex_buffers(std::vector<std::pair<std::shared_ptr<Buffer>, uint64_t>> vertex_buffers);

    void bind_index_buffer(Buffer *buffer);

    /// Bind uniform buffers and texture samplers.
    /// Image layout should be ready before calling this.
    void bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set);

    /// Draw call.
    void draw(uint32_t first_vertex, uint32_t vertex_count);

    /// We don't support vertex offset, as it's not supported by GLES 3.0.
    void draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, uint32_t first_instance);

    void draw_instanced(uint32_t vertex_count, uint32_t instance_count);

    void draw_indirect(Buffer *buffer, uint32_t offset);

    void end_render_pass();

    // COMPUTE PASS

    void begin_compute_pass();

    void bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline);

    void dispatch(uint32_t group_size_x, uint32_t group_size_y, uint32_t group_size_z);

    void dispatch_indirect(Buffer *buffer, uint32_t offset);

    void end_compute_pass();

    // DATA TRANSFER

    /// fixme: should set up a staging buffer every time we need to write to a buffer.
    /**
     * Upload to buffer.
     * @param buffer
     * @param offset
     * @param data_size Size of the data we are uploading, not the size of the buffer.
     * @param data
     */
    virtual void write_buffer(const std::shared_ptr<Buffer> &buffer,
                              uint32_t offset,
                              uint32_t data_size,
                              const void *data);

    /// fixme: should set up a staging buffer every time we need to read from a buffer.
    void read_buffer(const std::shared_ptr<Buffer> &buffer, uint32_t offset, uint32_t data_size, void *data);

    void write_texture(const std::shared_ptr<Texture> &texture, RectI region, const void *data);

    void read_texture(const std::shared_ptr<Texture> &texture, RectI region, void *data);

    void invoke_callbacks();

protected:
    CommandEncoder() = default;

    /// Prepare the encoder for submission.
    /// @return If valid for submission.
    virtual bool prepare() = 0;

    void add_callback(const std::function<void()> &callback) {
        callbacks_.push_back(callback);
    }

    void track_temporary_resource(const std::shared_ptr<Buffer> &buffer) {
        temp_buffers_.push_back(buffer);
    }

    /// Debug label.
    std::string label_;

    std::deque<Command> commands_;

    /// Prepared for submission.
    bool prepared_ = false;

    /// Submitted by a queue.
    bool submitted_ = false;

    /// Callbacks after the commands are finished (not only submitted) on GPU.
    std::vector<std::function<void()>> callbacks_;

    /// Temporary resources that should be kept alive until the command buffer is finished.
    std::vector<std::shared_ptr<Buffer>> temp_buffers_;

    /// Currently bound pipeline.
    RenderPipeline *render_pipeline_{};
    ComputePipeline *compute_pipeline_{};

    std::weak_ptr<Device> device_;

    std::vector<std::shared_ptr<Framebuffer>> framebuffers_;
};

} // namespace Pathfinder
