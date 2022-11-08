#include "driver.h"

#include "../../common/logger.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "validation.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

std::shared_ptr<Framebuffer> DriverGl::create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                          const std::shared_ptr<Texture> &texture,
                                                          const std::string &label) {
    auto framebuffer_gl = std::make_shared<FramebufferGl>(texture);

    check_error("create_framebuffer");
    return framebuffer_gl;
}

std::shared_ptr<Buffer> DriverGl::create_buffer(BufferType type,
                                                size_t size,
                                                MemoryProperty property,
                                                const std::string &label) {
    auto buffer = std::make_shared<BufferGl>(type, size, property, label);
    check_error("create_buffer");
    return buffer;
}

std::shared_ptr<Texture> DriverGl::create_texture(Vec2I size, TextureFormat format, const std::string &label) {
    auto texture_gl = std::make_shared<TextureGl>(size, format, label);
    check_error("create_texture");
    return texture_gl;
}

std::shared_ptr<CommandBuffer> DriverGl::create_command_buffer(const std::string &label) {
    auto command_buffer_gl = std::make_shared<CommandBufferGl>();
    check_error("create_command_buffer");
    return command_buffer_gl;
}

std::shared_ptr<RenderPass> DriverGl::create_render_pass(TextureFormat format,
                                                         AttachmentLoadOp load_op,
                                                         bool is_swapchain_render_pass,
                                                         const std::string &label) {
    auto render_pass_gl = std::make_shared<RenderPassGl>(load_op);
    check_error("create_render_pass");
    return render_pass_gl;
}

std::shared_ptr<RenderPipeline> DriverGl::create_render_pipeline(
    const std::vector<char> &vert_source,
    const std::vector<char> &frag_source,
    const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
    BlendState blend_state,
    const std::shared_ptr<DescriptorSet> &descriptor_set,
    const std::shared_ptr<RenderPass> &render_pass,
    const std::string &label) {
    auto pipeline_gl =
        std::make_shared<RenderPipelineGl>(vert_source, frag_source, attribute_descriptions, blend_state, label);
    check_error("create_render_pipeline");
    return pipeline_gl;
}

std::shared_ptr<DescriptorSet> DriverGl::create_descriptor_set() {
    return std::make_shared<DescriptorSet>();
}

std::shared_ptr<ComputePipeline> DriverGl::create_compute_pipeline(const std::vector<char> &comp_source,
                                                                   const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                                   const std::string &label) {
    auto pipeline_gl = std::make_shared<ComputePipelineGl>(comp_source);
    check_error("create_compute_pipeline");
    return pipeline_gl;
}

} // namespace Pathfinder

#endif
