#ifndef PATHFINDER_GPU_COMMAND_BUFFER_H
#define PATHFINDER_GPU_COMMAND_BUFFER_H

#include "render_pipeline.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "buffer.h"
#include "render_pass.h"
#include "descriptor_set.h"
#include "../common/color.h"
#include "../common/math/rect.h"

#include <cstdint>
#include <queue>
#include <memory>
#include <functional>
#include <array>

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

        // Compute pass.
        BeginComputePass,
        BindComputePipeline,
        Dispatch,
        EndComputePass,

        // Data transfer.
        UploadToBuffer,
        UploadToTexture,
        ReadBuffer,

        Max,
    };

    struct Command {
        CommandType type = CommandType::Max;

        union Args {
            struct {
                RenderPass *render_pass;
                Framebuffer *framebuffer;
                Vec2<uint32_t> extent;
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
            } upload_to_texture;
            struct {
                Buffer *buffer;
                uint32_t offset;
                uint32_t data_size;
                void *data;
            } read_buffer;
        } args;
    };

    class CommandBuffer {
    public:
        // RENDER PASS

        void begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                               const std::shared_ptr<Framebuffer> &framebuffer,
                               ColorF clear_color);

        /// Bind pipeline.
        void bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline);

        void bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers);

        /// Bind uniform buffers and texture samplers.
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
                               Rect<uint32_t> region,
                               const void *data);

        void read_buffer(const std::shared_ptr<Buffer> &buffer,
                         uint32_t offset,
                         uint32_t data_size,
                         void *data);

        // SUBMIT

        virtual void submit(const std::shared_ptr<Driver> &p_driver) = 0;

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
}

#endif //PATHFINDER_GPU_COMMAND_BUFFER_H
