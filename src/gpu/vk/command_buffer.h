#ifndef PATHFINDER_GPU_COMMAND_BUFFER_VK_H
#define PATHFINDER_GPU_COMMAND_BUFFER_VK_H

#include "../command_buffer.h"

#include <cstdint>
#include <queue>
#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class CommandBufferVk : public CommandBuffer {
        friend class DriverVk;
        friend class SwapChainVk;
    public:
        /// We have to provide these two to create a valid command buffer.
        CommandBufferVk(VkCommandBuffer command_buffer, VkDevice device);

        // Render pass

        void begin_render_pass(const std::shared_ptr<RenderPass> &render_pass,
                               const std::shared_ptr<Framebuffer> &framebuffer,
                               bool clear,
                               ColorF clear_color) override;

        /// Bind pipeline.
        void bind_render_pipeline(const std::shared_ptr<RenderPipeline> &pipeline) override;

        void bind_vertex_buffers(std::vector<std::shared_ptr<Buffer>> vertex_buffers) override;

        /// Bind uniform buffers and samplers.
        void bind_descriptor_set(const std::shared_ptr<DescriptorSet> &descriptor_set) override;

        /// Draw call.
        void draw(uint32_t first_vertex, uint32_t vertex_count) override;

        /// Instanced draw call.
        void draw_instanced(uint32_t vertex_count, uint32_t instance_count) override;

        void end_render_pass() override;

        // Compute pass

        void begin_compute_pass() override;

        void bind_compute_pipeline(const std::shared_ptr<ComputePipeline> &pipeline) override;

        void dispatch(uint32_t group_size_x, uint32_t group_size_y, uint32_t group_size_z) override;

        void end_compute_pass() override;

        // Data transfer

        /**
         * Upload to buffer.
         * @param buffer
         * @param offset
         * @param data_size Size of the data we are uploading, not the size of the buffer.
         * @param data
         */
        void upload_to_buffer(const std::shared_ptr<Buffer> &buffer, uint32_t offset, uint32_t data_size,
                              void *data) override;

        void upload_to_texture(const std::shared_ptr<Texture> &texture,
                               Rect<uint32_t> region,
                               const void *data) override;

        void read_buffer(const std::shared_ptr<Buffer> &buffer,
                         uint32_t offset,
                         uint32_t data_size,
                         void *data) override;

        // Submit

        void submit(const std::shared_ptr<Driver> &p_driver) override;

    private:
        VkCommandBuffer vk_command_buffer;

        VkDevice vk_device;
    };
}

#endif

#endif //PATHFINDER_GPU_COMMAND_BUFFER_VK_H
