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

namespace Pathfinder {
    class Driver;

    enum class CommandType {
        // Render pass.
        BeginRenderPass = 0,
        BindRenderPipeline,
        BindVertexBuffers,
        BindIndexBuffer,
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

        Submit,

        Max,
    };

    struct Command {
        CommandType type = CommandType::Max;

        union Args {
            struct {
                RenderPass *render_pass;
                Framebuffer *framebuffer;
                Vec2<uint32_t> extent;
                bool clear;
                ColorF clear_color;
            } begin_render_pass{};
            struct {
                RenderPipeline *pipeline;
            } bind_render_pipeline;
            struct {
                uint32_t buffer_count;
                std::array<Buffer *, 10> buffers;
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

        virtual void begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                                       const std::shared_ptr<Framebuffer> &framebuffer,
                                       bool clear,
                                       ColorF clear_color) = 0;

        /// Bind pipeline.
        virtual void bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline) = 0;

        virtual void bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers) = 0;

        /// Bind uniform buffers and texture samplers.
        virtual void bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set) = 0;

        /// Draw call.
        virtual void draw(uint32_t first_vertex, uint32_t vertex_count) = 0;

        virtual void draw_instanced(uint32_t vertex_count, uint32_t instance_count) = 0;

        virtual void end_render_pass() = 0;

        // COMPUTE PASS

        virtual void begin_compute_pass() = 0;

        virtual void bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline) = 0;

        virtual void dispatch(uint32_t group_size_x, uint32_t group_size_y, uint32_t group_size_z) = 0;

        virtual void end_compute_pass() = 0;

        // DATA TRANSFER

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

        virtual void upload_to_texture(const std::shared_ptr<Texture> &texture,
                                       Rect<uint32_t> region,
                                       const void *data) = 0;

        virtual void read_buffer(const std::shared_ptr<Buffer> &buffer,
                                 uint32_t offset,
                                 uint32_t data_size,
                                 void *data) = 0;

        // SUBMIT

        virtual void submit(const std::shared_ptr<Driver> &p_driver) = 0;

    protected:
        std::queue<Command> commands;

        RenderPipeline *render_pipeline{};
        ComputePipeline *compute_pipeline{};
    };
}

#endif //PATHFINDER_GPU_COMMAND_BUFFER_H
