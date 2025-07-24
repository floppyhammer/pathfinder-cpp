#ifndef PATHFINDER_GPU_DEVICE_H
#define PATHFINDER_GPU_DEVICE_H

#include "../common/logger.h"
#include "buffer.h"
#include "command_encoder.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "shader_module.h"

namespace Pathfinder {

enum class BackendType {
    Opengl,
    Vulkan,
};

/// We only need to provide a Driver to Canvas for rendering,
/// which means Window and SwapChain aren't needed for platforms like Android.
class Device : public std::enable_shared_from_this<Device> {
public:
    virtual ~Device() = default;

    virtual std::shared_ptr<Framebuffer> create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                            const std::shared_ptr<Texture> &texture,
                                                            const std::string &label) = 0;

    virtual std::shared_ptr<Buffer> create_buffer(const BufferDescriptor &desc, const std::string &label) = 0;

    virtual std::shared_ptr<Texture> create_texture(const TextureDescriptor &desc, const std::string &label) = 0;

    virtual std::shared_ptr<Sampler> create_sampler(SamplerDescriptor descriptor) = 0;

    virtual std::shared_ptr<CommandEncoder> create_command_encoder(const std::string &label) = 0;

    virtual std::shared_ptr<DescriptorSet> create_descriptor_set() = 0;

    virtual std::shared_ptr<RenderPass> create_render_pass(TextureFormat format,
                                                           AttachmentLoadOp load_op,
                                                           const std::string &label) = 0;

    virtual std::shared_ptr<RenderPass> create_swap_chain_render_pass(TextureFormat format,
                                                                      AttachmentLoadOp load_op) = 0;

    virtual std::shared_ptr<ShaderModule> create_shader_module(const std::vector<char> &source_code,
                                                               ShaderStage shader_stage,
                                                               const std::string &label) = 0;

    virtual std::shared_ptr<RenderPipeline> create_render_pipeline(
        const std::shared_ptr<ShaderModule> &vert_shader_module,
        const std::shared_ptr<ShaderModule> &frag_shader_module,
        const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
        BlendState blend_state,
        const std::shared_ptr<DescriptorSet> &descriptor_set,
        TextureFormat target_format,
        const std::string &label) = 0;

    virtual std::shared_ptr<ComputePipeline> create_compute_pipeline(
        const std::shared_ptr<ShaderModule> &comp_shader_module,
        const std::shared_ptr<DescriptorSet> &descriptor_set,
        const std::string &label) = 0;

    BackendType get_backend_type() const {
        return backend_type;
    }

protected:
    BackendType backend_type;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DEVICE_H
