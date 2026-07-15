#pragma once

#import <Metal/Metal.h>

#include "../device.h"
#include "../queue.h"

namespace Pathfinder {

class TextureMtl;
class FramebufferMtl;
class QueueMtl;

class DeviceMtl final : public Device {
public:
    DeviceMtl(id<MTLDevice> device, id<MTLCommandQueue> mtl_cmd_queue);

    ~DeviceMtl() override;

    std::shared_ptr<Framebuffer> create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                    const std::shared_ptr<Texture> &texture,
                                                    const std::string &label) override;

    std::shared_ptr<Buffer> create_buffer(const BufferDescriptor &descriptor, const std::string &label) override;

    std::shared_ptr<Texture> create_texture(const TextureDescriptor &descriptor, const std::string &label) override;

    std::shared_ptr<Sampler> create_sampler(SamplerDescriptor descriptor) override;

    std::shared_ptr<CommandEncoder> create_command_encoder(const std::string &label) override;

    std::shared_ptr<DescriptorSetLayout> create_descriptor_set_layout(
        const std::vector<DescriptorLayout> &descriptors) override;

    std::shared_ptr<DescriptorSet> create_descriptor_set(std::shared_ptr<DescriptorSetLayout> layout) override;

    std::shared_ptr<RenderPass> create_render_pass(TextureFormat format,
                                                   AttachmentLoadOp load_op,
                                                   const std::string &label) override;

    std::shared_ptr<RenderPass> create_swap_chain_render_pass(TextureFormat format, AttachmentLoadOp load_op) override;

    std::shared_ptr<ShaderModule> create_shader_module(const std::shared_ptr<Shader> &shader,
                                                       const std::string &label) override;

    std::shared_ptr<ShaderModule> create_shader_module(const std::vector<char> &source_code,
                                                       ShaderStage shader_stage,
                                                       const std::string &label) override {
        // Do nothing
        return nullptr;
    }

    std::shared_ptr<RenderPipeline> create_render_pipeline(
        const std::shared_ptr<ShaderModule> &vert_shader_module,
        const std::shared_ptr<ShaderModule> &frag_shader_module,
        const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
        BlendState blend_state,
        const std::shared_ptr<DescriptorSetLayout> &descriptor_set_layout,
        TextureFormat target_format,
        const std::string &label) override;

    std::shared_ptr<ComputePipeline> create_compute_pipeline(
        const std::shared_ptr<ShaderModule> &comp_shader_module,
        const std::shared_ptr<DescriptorSetLayout> &descriptor_set_layout,
        const std::string &label) override;

    std::shared_ptr<Fence> create_fence(const std::string &label) override;

    void *map_staging(const StagingAllocation &allocation) override;

    void unmap_staging(const StagingAllocation &allocation) override;

    std::shared_ptr<Buffer> create_staging_buffer(size_t size) override;

    size_t get_aligned_uniform_size(size_t original_size) override;

    std::shared_ptr<TextureMtl> wrap_texture(id<MTLTexture> mtl_texture, bool has_resource_ownership);

    std::shared_ptr<QueueMtl> wrap_queue(id<MTLCommandQueue> mtl_cmd_queue, bool has_resource_ownership);

    std::shared_ptr<QueueMtl> get_queue() {
        return queue_;
    }

    id<MTLDevice> get_handle() {
        return mtl_device_;
    }

private:
    id<MTLDevice> mtl_device_ = nil;
    id<MTLCommandQueue> mtl_cmd_queue_ = nil;
    std::shared_ptr<QueueMtl> queue_;
};

} // namespace Pathfinder
