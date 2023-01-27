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
    return std::make_shared<FramebufferGl>(texture, label);
}

std::shared_ptr<Buffer> DriverGl::create_buffer(const BufferDescriptor &desc) {
    return std::make_shared<BufferGl>(desc);
}

std::shared_ptr<Texture> DriverGl::create_texture(const TextureDescriptor &desc) {
    return std::make_shared<TextureGl>(desc);
}

std::shared_ptr<CommandBuffer> DriverGl::create_command_buffer(const std::string &label) {
    return std::make_shared<CommandBufferGl>();
}

std::shared_ptr<RenderPass> DriverGl::create_render_pass(TextureFormat format,
                                                         AttachmentLoadOp load_op,
                                                         const std::string &label) {
    return std::make_shared<RenderPassGl>(load_op);
}

std::shared_ptr<RenderPass> DriverGl::create_swap_chain_render_pass(TextureFormat format, AttachmentLoadOp load_op) {
    return std::make_shared<RenderPassGl>(load_op);
}

std::shared_ptr<RenderPipeline> DriverGl::create_render_pipeline(
    const std::vector<char> &vert_source,
    const std::vector<char> &frag_source,
    const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
    BlendState blend_state,
    const std::shared_ptr<DescriptorSet> &descriptor_set,
    const std::shared_ptr<RenderPass> &render_pass,
    const std::string &label) {
    return std::make_shared<RenderPipelineGl>(vert_source, frag_source, attribute_descriptions, blend_state, label);
}

std::shared_ptr<DescriptorSet> DriverGl::create_descriptor_set() {
    return std::make_shared<DescriptorSet>();
}

std::shared_ptr<ComputePipeline> DriverGl::create_compute_pipeline(const std::vector<char> &comp_source,
                                                                   const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                                   const std::string &label) {
    return std::make_shared<ComputePipelineGl>(comp_source);
}

} // namespace Pathfinder

#endif
