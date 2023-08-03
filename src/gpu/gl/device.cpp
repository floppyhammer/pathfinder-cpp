#include "device.h"

#include "../../common/logger.h"
#include "compute_pipeline.h"
#include "debug_marker.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

std::shared_ptr<Framebuffer> DeviceGl::create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                          const std::shared_ptr<Texture> &texture,
                                                          const std::string &label) {
    auto framebuffer_gl = std::shared_ptr<FramebufferGl>(new FramebufferGl(texture));
    framebuffer_gl->set_label(label);
    return framebuffer_gl;
}

std::shared_ptr<Buffer> DeviceGl::create_buffer(const BufferDescriptor &desc, const std::string &label) {
    auto buffer_gl = std::shared_ptr<BufferGl>(new BufferGl(desc));
    buffer_gl->set_label(label);
    return buffer_gl;
}

std::shared_ptr<Texture> DeviceGl::create_texture(const TextureDescriptor &desc, const std::string &label) {
    auto texture_gl = std::shared_ptr<TextureGl>(new TextureGl(desc));
    texture_gl->set_label(label);
    return texture_gl;
}

std::shared_ptr<Sampler> DeviceGl::create_sampler(SamplerDescriptor descriptor) {
    return std::shared_ptr<Sampler>(new Sampler(descriptor));
}

std::shared_ptr<CommandBuffer> DeviceGl::create_command_buffer(const std::string &label) {
    return std::shared_ptr<CommandBufferGl>(new CommandBufferGl());
}

std::shared_ptr<RenderPass> DeviceGl::create_render_pass(TextureFormat format,
                                                         AttachmentLoadOp load_op,
                                                         const std::string &label) {
    return std::shared_ptr<RenderPassGl>(new RenderPassGl(load_op));
}

std::shared_ptr<RenderPass> DeviceGl::create_swap_chain_render_pass(TextureFormat format, AttachmentLoadOp load_op) {
    return std::shared_ptr<RenderPassGl>(new RenderPassGl(load_op));
}

std::shared_ptr<RenderPipeline> DeviceGl::create_render_pipeline(
    const std::vector<char> &vert_source,
    const std::vector<char> &frag_source,
    const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
    BlendState blend_state,
    const std::shared_ptr<DescriptorSet> &descriptor_set,
    const std::shared_ptr<RenderPass> &render_pass,
    const std::string &label) {
    return std::shared_ptr<RenderPipelineGl>(
        new RenderPipelineGl(vert_source, frag_source, attribute_descriptions, blend_state, label));
}

std::shared_ptr<DescriptorSet> DeviceGl::create_descriptor_set() {
    return std::shared_ptr<DescriptorSet>(new DescriptorSet());
}

std::shared_ptr<ComputePipeline> DeviceGl::create_compute_pipeline(const std::vector<char> &comp_source,
                                                                   const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                                   const std::string &label) {
    return std::shared_ptr<ComputePipelineGl>(new ComputePipelineGl(comp_source));
}

} // namespace Pathfinder

#endif
