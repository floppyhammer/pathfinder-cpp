#include "device.h"

#include "render_pipeline.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "validation.h"

#include "../../common/logger.h"

namespace Pathfinder {
    std::shared_ptr<Framebuffer> DeviceGl::create_framebuffer(uint32_t p_width, uint32_t p_height,
                                                              TextureFormat p_format, DataType p_type) {
        auto framebuffer = std::make_shared<FramebufferGl>(p_width, p_height, p_format, p_type);

        check_error("create_framebuffer");
        return framebuffer;
    }

    std::shared_ptr<Buffer> DeviceGl::create_buffer(BufferType type, size_t size) {
        auto buffer = std::make_shared<BufferGl>(type, size);

        check_error("create_buffer");
        return buffer;
    }

    std::shared_ptr<Texture> DeviceGl::create_texture(uint32_t p_width,
                                                      uint32_t p_height,
                                                      TextureFormat p_format,
                                                      DataType p_type) {
        auto texture = std::make_shared<TextureGl>(p_width, p_height, p_format, p_type);

        check_error("create_texture");
        return texture;
    }

    std::shared_ptr<CommandBuffer> DeviceGl::create_command_buffer() {
        auto framebuffer = std::make_shared<CommandBufferGl>();

        check_error("create_command_buffer");
        return framebuffer;
    }

    std::shared_ptr<RenderPipeline> DeviceGl::create_render_pipeline(const std::string &vert_source,
                                                                     const std::string &frag_source,
                                                                     const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                                                                     ColorBlendState blend_state) {
        auto pipeline_gl = std::make_shared<RenderPipelineGl>(vert_source,
                                                           frag_source,
                                                           attribute_descriptions,
                                                           blend_state);

        check_error("create_render_pipeline");
        return pipeline_gl;
    }

    std::shared_ptr<ComputePipeline> DeviceGl::create_compute_pipeline(const std::string &comp_source) {
        auto pipeline_gl = std::make_shared<ComputePipelineGl>(comp_source);

        check_error("create_compute_pipeline");
        return pipeline_gl;
    }
}
