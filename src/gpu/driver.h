#ifndef PATHFINDER_GPU_DRIVER_H
#define PATHFINDER_GPU_DRIVER_H

#include "buffer.h"
#include "command_buffer.h"
#include "compute_pipeline.h"
#include "framebuffer.h"
#include "render_pass.h"
#include "render_pipeline.h"
#include "swap_chain.h"

namespace Pathfinder {
/// We only need to provide a Driver to Canvas for Pathfinder rendering,
/// which means Platform and SwapChain aren't needed for platforms like Android.
class Driver {
public:
    virtual std::shared_ptr<Framebuffer> create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                            const std::shared_ptr<Texture> &texture) = 0;

    virtual std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size, MemoryProperty property) = 0;

    virtual std::shared_ptr<Texture> create_texture(uint32_t p_width, uint32_t p_height, TextureFormat p_format) = 0;

    virtual std::shared_ptr<CommandBuffer> create_command_buffer(bool one_time) = 0;

    virtual std::shared_ptr<DescriptorSet> create_descriptor_set() = 0;

    virtual std::shared_ptr<RenderPass> create_render_pass(TextureFormat format,
                                                           AttachmentLoadOp load_op,
                                                           TextureLayout final_layout) = 0;

    virtual std::shared_ptr<RenderPipeline> create_render_pipeline(
        const std::vector<char> &vert_source,
        const std::vector<char> &frag_source,
        const std::vector<VertexInputAttributeDescription> &p_attribute_descriptions,
        ColorBlendState p_blend_state,
        const std::shared_ptr<DescriptorSet> &descriptor_set,
        const std::shared_ptr<RenderPass> &render_pass) = 0;

    virtual std::shared_ptr<ComputePipeline> create_compute_pipeline(
        const std::vector<char> &comp_source, const std::shared_ptr<DescriptorSet> &descriptor_set) = 0;
};
} // namespace Pathfinder

#endif // PATHFINDER_GPU_DRIVER_H
