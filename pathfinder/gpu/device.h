#pragma once

#include "../common/logger.h"
#include "buffer.h"
#include "command_encoder.h"
#include "compute_pipeline.h"
#include "fence.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "shader.h"
#include "shader_module.h"

namespace Pathfinder {

enum class BackendType {
    Opengl,
    Vulkan,
    Metal,
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

    virtual std::shared_ptr<DescriptorSetLayout> create_descriptor_set_layout(
        const std::vector<DescriptorLayout> &descriptors) = 0;

    virtual std::shared_ptr<DescriptorSet> create_descriptor_set(std::shared_ptr<DescriptorSetLayout> layout) = 0;

    virtual std::shared_ptr<RenderPass> create_render_pass(TextureFormat format,
                                                           AttachmentLoadOp load_op,
                                                           const std::string &label) = 0;

    virtual std::shared_ptr<RenderPass> create_swap_chain_render_pass(TextureFormat format,
                                                                      AttachmentLoadOp load_op) = 0;

    virtual std::shared_ptr<ShaderModule> create_shader_module(const std::shared_ptr<Shader> &shader,
                                                               const std::string &label) = 0;

    virtual std::shared_ptr<ShaderModule> create_shader_module(const std::vector<char> &source_code,
                                                               ShaderStage shader_stage,
                                                               const std::string &label) = 0;

    virtual std::shared_ptr<RenderPipeline> create_render_pipeline(
        const std::shared_ptr<ShaderModule> &vert_shader_module,
        const std::shared_ptr<ShaderModule> &frag_shader_module,
        const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
        BlendState blend_state,
        const std::shared_ptr<DescriptorSetLayout> &descriptor_set_layout,
        TextureFormat target_format,
        const std::string &label) = 0;

    virtual std::shared_ptr<ComputePipeline> create_compute_pipeline(
        const std::shared_ptr<ShaderModule> &comp_shader_module,
        const std::shared_ptr<DescriptorSetLayout> &descriptor_set_layout,
        const std::string &label) = 0;

    virtual std::shared_ptr<Fence> create_fence(const std::string &label) = 0;

    StagingAllocation allocate_staging(size_t size);

    virtual void *map_staging(const StagingAllocation &allocation) {
        return nullptr;
    }

    virtual void unmap_staging(const StagingAllocation &allocation) {}

    virtual void reset_staging();

    virtual size_t get_aligned_uniform_size(size_t original_size) = 0;

    BackendType get_backend_type() const {
        return backend_type;
    }

public:
    void increment_staging_encoder() {
        pending_staging_encoders_++;
    }

    void decrement_staging_encoder() {
        pending_staging_encoders_--;
        if (pending_staging_encoders_ == 0) {
            reset_staging();
        }
    }

protected:
    BackendType backend_type = BackendType::Vulkan;

    struct StagingBlock {
        std::shared_ptr<Buffer> buffer;
        size_t used_size = 0;
    };

    const size_t STAGING_BLOCK_SIZE = 4 * 1024 * 1024;

    std::vector<StagingBlock> staging_blocks_;

    uint32_t pending_staging_encoders_ = 0;

    virtual std::shared_ptr<Buffer> create_staging_buffer(size_t size) = 0;
};

} // namespace Pathfinder
