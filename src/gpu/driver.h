#ifndef PATHFINDER_GPU_DRIVER_H
#define PATHFINDER_GPU_DRIVER_H

#include "swap_chain.h"
#include "render_pass.h"
#include "framebuffer.h"
#include "buffer.h"
#include "command_buffer.h"
#include "render_pipeline.h"
#include "compute_pipeline.h"

namespace Pathfinder {
    class Driver {
    public:
        virtual std::shared_ptr<Framebuffer> create_framebuffer(uint32_t p_width,
                                                                uint32_t p_height,
                                                                TextureFormat p_format,
                                                                const std::shared_ptr<RenderPass> &render_pass) = 0;

        virtual std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size, MemoryProperty property) = 0;

        virtual std::shared_ptr<Texture> create_texture(uint32_t p_width,
                                                        uint32_t p_height,
                                                        TextureFormat p_format) = 0;

        virtual std::shared_ptr<CommandBuffer> create_command_buffer(bool one_time) = 0;

        virtual std::shared_ptr<DescriptorSet> create_descriptor_set() = 0;

        virtual std::shared_ptr<RenderPass> create_render_pass(TextureFormat format, ImageLayout final_layout) = 0;

        virtual std::shared_ptr<RenderPipeline> create_render_pipeline(const std::vector<char> &vert_source,
                                                                       const std::vector<char> &frag_source,
                                                                       const std::vector<VertexInputAttributeDescription> &p_attribute_descriptions,
                                                                       ColorBlendState p_blend_state,
                                                                       Vec2<uint32_t> viewport_size,
                                                                       const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                                       const std::shared_ptr<RenderPass> &render_pass) = 0;

        virtual std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::vector<char> &comp_source,
                                                                         const std::shared_ptr<DescriptorSet> &descriptor_set) = 0;
    };
}

#endif //PATHFINDER_GPU_DRIVER_H
