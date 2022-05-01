#include "driver.h"

#include "render_pipeline.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "validation.h"
#include "render_pass.h"

#include "../../common/logger.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    std::shared_ptr<Framebuffer> DriverGl::create_framebuffer(uint32_t p_width, uint32_t p_height,
                                                              TextureFormat p_format,
                                                              const std::shared_ptr<RenderPass> &render_pass) {
        auto framebuffer_gl = std::make_shared<FramebufferGl>(p_width, p_height, p_format);

        check_error("create_framebuffer");
        return framebuffer_gl;
    }

    std::shared_ptr<Buffer> DriverGl::create_buffer(BufferType type, size_t size) {
        auto buffer = std::make_shared<BufferGl>(type, size);

        check_error("create_buffer");
        return buffer;
    }

    std::shared_ptr<Texture> DriverGl::create_texture(uint32_t p_width,
                                                      uint32_t p_height,
                                                      TextureFormat p_format) {
        auto texture_gl = std::make_shared<TextureGl>(p_width, p_height, p_format);

        check_error("create_texture");
        return texture_gl;
    }

    std::shared_ptr<CommandBuffer> DriverGl::create_command_buffer(bool one_time) {
        auto command_buffer_gl = std::make_shared<CommandBufferGl>();

        check_error("create_command_buffer");
        return command_buffer_gl;
    }

    std::shared_ptr<RenderPass> DriverGl::create_render_pass(TextureFormat format, ImageLayout final_layout) {
        auto render_pass_gl = std::make_shared<RenderPassGl>();

        check_error("create_render_pass");
        return render_pass_gl;
    }

    std::shared_ptr<RenderPipeline> DriverGl::create_render_pipeline(const std::vector<char> &vert_source,
                                                                     const std::vector<char> &frag_source,
                                                                     const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                                                                     ColorBlendState blend_state,
                                                                     Vec2<uint32_t> viewport_size,
                                                                     const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                                     const std::shared_ptr<RenderPass> &render_pass) {
        auto pipeline_gl = std::make_shared<RenderPipelineGl>(vert_source,
                                                              frag_source,
                                                              attribute_descriptions,
                                                              blend_state);

        check_error("create_render_pipeline");
        return pipeline_gl;
    }

    std::shared_ptr<DescriptorSet> DriverGl::create_descriptor_set() {
        return std::make_shared<DescriptorSet>();
    }

    std::shared_ptr<ComputePipeline> DriverGl::create_compute_pipeline(const std::vector<char> &comp_source,
                                                                       const std::shared_ptr<DescriptorSet> &descriptor_set) {
        auto pipeline_gl = std::make_shared<ComputePipelineGl>(comp_source);

        check_error("create_compute_pipeline");
        return pipeline_gl;
    }
}

#endif
