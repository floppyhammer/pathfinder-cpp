#ifndef PATHFINDER_GPU_COMMAND_BUFFER_H
#define PATHFINDER_GPU_COMMAND_BUFFER_H

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

class Driver;

enum class CommandType {
    // Render pass.

    BeginRenderPass = 0,
    BindRenderPipeline,
    BindVertexBuffers,
    BindDescriptorSet,
    Draw,
    DrawInstanced,
    EndRenderPass,

    // This shouldn't happen during a render pass.
    SyncDescriptorSet,

    // Compute pass.

    BeginComputePass,
    BindComputePipeline,
    Dispatch,
    EndComputePass,

    // Data transfer.

    UploadToBuffer,
    UploadToTexture,
    ReadBuffer,
    //    TransitionLayout,

    Max,
};

struct Command {
    CommandType type = CommandType::Max;

    union Args {
        struct {
            RenderPass *render_pass;
            Framebuffer *framebuffer;
            Vec2I extent;
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
            DescriptorSet *descriptor_set;
        } sync_descriptor_set;
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
            void *data;
        } upload_to_buffer;
        struct {
            Texture *texture;
            uint32_t offset_x;
            uint32_t offset_y;
            uint32_t width;
            uint32_t height;
            uint32_t data_size;
            const void *data;
            TextureLayout dst_layout; // Final layout to put this texture in after the data copy.
        } upload_to_texture;
        struct {
            Buffer *buffer;
            uint32_t offset;
            uint32_t data_size;
            void *data;
        } read_buffer;
        //        struct {
        //            Texture *texture;
        //            TextureLayout dst_layout;
        //        } transition_layout;
    } args;
};

/// Don't do any modifications to command arguments until the buffer is submitted.
/// Maybe this command buffer will not be executed at all.
/// Or maybe a later generated command buffer will be executed first.
class CommandBuffer {
public:
    // For debug reason.
    std::string name = "Unnamed command buffer";

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

    void sync_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set);

    // COMPUTE PASS

    void begin_compute_pass();

    void bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline);

    void dispatch(uint32_t group_size_x, uint32_t group_size_y, uint32_t group_size_z);

    void end_compute_pass();

    // COPY PASS

    /**
     * Upload to buffer.
     * @param buffer
     * @param offset
     * @param data_size Size of the data we are uploading, not the size of the buffer.
     * @param data
     */
    virtual void upload_to_buffer(const std::shared_ptr<Buffer> &buffer,
                                  uint32_t offset,
                                  uint32_t data_size,
                                  void *data) = 0;

    void upload_to_texture(const std::shared_ptr<Texture> &texture,
                           RectI region,
                           const void *data,
                           TextureLayout dst_layout);

    void read_buffer(const std::shared_ptr<Buffer> &buffer, uint32_t offset, uint32_t data_size, void *data);

    // SUBMIT

    virtual void submit(const std::shared_ptr<Driver> &_driver) = 0;

    inline void add_callback(const std::function<void()> &callback) {
        callbacks.push_back(callback);
    }

protected:
    std::queue<Command> commands;

    bool one_time = false;

    /// Callbacks to call when the commands are flushed.
    std::vector<std::function<void()>> callbacks;

    /// Currently bind pipeline.
    RenderPipeline *render_pipeline{};
    ComputePipeline *compute_pipeline{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMMAND_BUFFER_H
