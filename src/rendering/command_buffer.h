//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_COMMAND_BUFFER_H
#define PATHFINDER_COMMAND_BUFFER_H

#include "raster_program.h"
#include "compute_program.h"
#include "../common/color.h"
#include "render_pipeline.h"
#include "compute_pipeline.h"
#include "buffer.h"
#include "descriptor_set.h"

#include <cstdint>
#include <queue>
#include <memory>

namespace Pathfinder {
    enum class CommandType {
        // Render
        BeginRenderPass,
        BindRenderPipeline,
        BindVertexBuffers,
        BindIndexBuffer,
        BindDescriptorSet,
        Draw,
        DrawInstanced,
        EndRenderPass,

        // Compute
        BeginComputePass,
        BindComputePipeline,
        Dispatch,
        EndComputePass,

        // Data
        UploadGeneralBuffer,
        ReadGeneralBuffer,
        UploadTexture,

        Max,
    };

    struct Command {
        CommandType type = CommandType::Max;

        union Args {
            struct {
//                std::shared_ptr<RenderPass> render_pass;
                uint32_t framebuffer_id;
                Vec2<uint32_t> extent;
                bool clear;
                ColorF clear_color;
            } begin_render_pass{};
            struct {
                RenderPipeline *pipeline;
            } bind_render_pipeline;
            struct {
                uint32_t buffer_count;
                std::array<Buffer*, 10> buffers;
            } bind_vertex_buffers;
            struct {
                uint32_t buffer_id;
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
        } args;
    };

    class CommandBuffer {
    public:
        void begin_render_pass(uint32_t framebuffer_id,
                               Vec2<uint32_t> extent,
                               bool clear,
                               ColorF clear_color);

        // Bind pipeline.
        void bind_render_pipeline(const std::shared_ptr<RenderPipeline>& pipeline);

        void bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers);

        // Bind uniform buffers and samplers.
        void bind_descriptor_set(const std::shared_ptr<DescriptorSet>& descriptor_set);

        // Draw call.
        void draw(uint32_t first_vertex, uint32_t vertex_count);
        void draw_instanced(uint32_t vertex_count, uint32_t instance_count);

        void end_render_pass();

        void begin_compute_pass() {};

        void bind_compute_pipeline(const std::shared_ptr<ComputePipeline>& pipeline);

        void dispatch(uint32_t group_size_x = 1, uint32_t group_size_y = 1, uint32_t group_size_z = 1);

        void end_compute_pass();

        // Data

        void submit();

    private:
        std::queue<Command> commands;

        Pipeline *current_pipeline;
    };
}

#endif //PATHFINDER_COMMAND_BUFFER_H
