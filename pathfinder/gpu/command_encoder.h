#ifndef PATHFINDER_GPU_COMMAND_BUFFER_H
#define PATHFINDER_GPU_COMMAND_BUFFER_H

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>

#include "../common/color.h"
#include "../common/logger.h"
#include "../common/math/rect.h"
#include "buffer.h"
#include "compute_pipeline.h"
#include "descriptor_set.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"

namespace Pathfinder {

enum class CommandType {
    // RENDER PASS

    BeginRenderPass = 0,
    BindRenderPipeline,
    BindVertexBuffers,
    BindDescriptorSet,
    Draw,
    DrawInstanced,
    EndRenderPass,

    // COMPUTE PASS

    BeginComputePass,
    BindComputePipeline,
    Dispatch,
    EndComputePass,

    // DATA TRANSFER

    WriteBuffer,
    ReadBuffer,
    WriteTexture,
    ReadTexture,

    Max,
};

struct Command {
    CommandType type = CommandType::Max;

    union Args {
        struct {
            RenderPass *render_pass;
            Framebuffer *framebuffer;
            RectI viewport;
            ColorF clear_color;
        } begin_render_pass{};
        struct {
            RenderPipeline *pipeline;
        } bind_render_pipeline;
        struct {
            uint32_t buffer_count;
            std::array<Buffer *, MAX_VERTEX_BUFFER_BINDINGS> buffers;
        } bind_vertex_buffers;
        struct {
            Buffer *buffer;
        } bind_index_buffer;
        struct {
            DescriptorSet *descriptor_set;
        } bind_descriptor_set;
        struct {
            uint32_t first_vertex;
            uint32_t vertex_count;
        } draw;
        struct {
            uint32_t vertex_count;
            uint32_t instance_count;
        } draw_instanced;
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
        } write_buffer;
        struct {
            Buffer *buffer;
            uint32_t offset;
            uint32_t data_size;
            void *data;
        } read_buffer;
        struct {
            Texture *texture;
            uint32_t offset_x;
            uint32_t offset_y;
            uint32_t width;
            uint32_t height;
            const void *data;
        } write_texture;
        struct {
            Texture *texture;
            uint32_t offset_x;
            uint32_t offset_y;
            uint32_t width;
            uint32_t height;
            void *data;
        } read_texture;
    } args;
};

/// Don't do any modifications to command arguments until the buffer is submitted.
/// Maybe this command buffer will not be executed at all.
/// Or maybe a later generated command buffer will be executed first.
class CommandEncoder {
    friend class QueueVk;
    friend class QueueGl;
    friend class SwapChainVk;

public:
    virtual ~CommandEncoder() = default;

    // RENDER PASS

    void begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                           const std::shared_ptr<Framebuffer> &framebuffer,
                           ColorF clear_color);

    /// Bind pipeline.
    void bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline);

    void bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers);

    /// Bind uniform buffers and texture samplers.
    /// Image layout should be ready before calling this.
    void bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set);

    /// Draw call.
    void draw(uint32_t first_vertex, uint32_t vertex_count);

    void draw_instanced(uint32_t vertex_count, uint32_t instance_count);

    void end_render_pass();

    // COMPUTE PASS

    void begin_compute_pass();

    void bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline);

    void dispatch(uint32_t group_size_x, uint32_t group_size_y, uint32_t group_size_z);

    void end_compute_pass();

    // DATA TRANSFER

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

    void read_buffer(const std::shared_ptr<Buffer> &buffer, uint32_t offset, uint32_t data_size, void *data);

    void write_texture(const std::shared_ptr<Texture> &texture, RectI region, const void *data);

    void read_texture(const std::shared_ptr<Texture> &texture, RectI region, void *data);

    void add_callback(const std::function<void()> &callback) {
        callbacks_.push_back(callback);
    }

    void invoke_callbacks() {
        for (auto &callback : callbacks_) {
            callback();
        }

        callbacks_.clear();
    }

protected:
    CommandEncoder() = default;

    /// Prepare the encoder for submission.
    /// @return If valid for submission.
    virtual bool finish() = 0;

    /// Debug label.
    std::string label_;

    std::deque<Command> commands_;

    /// Finished recording.
    bool finished_ = false;

    /// Submitted by a queue.
    bool submitted_ = false;

    /// Callbacks after the commands are submitted and waited for finish.
    std::vector<std::function<void()>> callbacks_;

    /// Currently bound pipeline.
    RenderPipeline *render_pipeline_{};
    ComputePipeline *compute_pipeline_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMMAND_BUFFER_H
