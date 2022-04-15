//
// Created by chy on 4/15/2022.
//

#ifndef PATHFINDER_DEVICE_H
#define PATHFINDER_DEVICE_H

#include "gl/framebuffer.h"
#include "gl/buffer.h"
#include "gl/command_buffer.h"
#include "render_pipeline.h"
#include "compute_pipeline.h"

namespace Pathfinder {
    class Device {
    public:
        virtual std::shared_ptr<Framebuffer> create_framebuffer(uint32_t p_width,
                                                                uint32_t p_height,
                                                                TextureFormat p_format,
                                                                DataType p_type) = 0;

        virtual std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size) = 0;

        virtual std::shared_ptr<Texture> create_texture(uint32_t p_width,
                                                        uint32_t p_height,
                                                        TextureFormat p_format,
                                                        DataType p_type) = 0;

        virtual std::shared_ptr<CommandBuffer> create_command_buffer() = 0;

        virtual std::shared_ptr<RenderPipeline> create_render_pipeline(const std::string &vert_source,
                                                                       const std::string &frag_source,
                                                                       const std::vector<VertexInputAttributeDescription> &p_attribute_descriptions,
                                                                       ColorBlendState p_blend_state) = 0;

        virtual std::shared_ptr<ComputePipeline> create_compute_pipeline() = 0;
    };
}

#endif //PATHFINDER_DEVICE_H
