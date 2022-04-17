#ifndef PATHFINDER_HAL_DEVICE_H
#define PATHFINDER_HAL_DEVICE_H

#include "render_pass.h"
#include "framebuffer.h"
#include "buffer.h"
#include "command_buffer.h"
#include "render_pipeline.h"
#include "compute_pipeline.h"

namespace Pathfinder {
    class Device {
    public:
        virtual std::shared_ptr<Framebuffer> create_framebuffer(uint32_t p_width,
                                                                uint32_t p_height,
                                                                TextureFormat p_format,
                                                                DataType p_type,
                                                                const std::shared_ptr<RenderPass> &render_pass) = 0;

        virtual std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size) = 0;

        virtual std::shared_ptr<Texture> create_texture(uint32_t p_width,
                                                        uint32_t p_height,
                                                        TextureFormat p_format,
                                                        DataType p_type) = 0;

        virtual std::shared_ptr<CommandBuffer> create_command_buffer() = 0;

        virtual std::shared_ptr<RenderPass> create_render_pass() = 0;

        virtual std::shared_ptr<RenderPipeline> create_render_pipeline(const std::vector<char> &vert_source,
                                                                       const std::vector<char> &frag_source,
                                                                       const std::vector<VertexInputAttributeDescription> &p_attribute_descriptions,
                                                                       ColorBlendState p_blend_state,
                                                                       const std::shared_ptr<RenderPass> &render_pass) = 0;

        virtual std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::vector<char> &comp_source) = 0;
    };
}

#endif //PATHFINDER_HAL_DEVICE_H
