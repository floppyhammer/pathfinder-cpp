#ifndef PATHFINDER_GPU_DRIVER_GL_H
#define PATHFINDER_GPU_DRIVER_GL_H

#include <vector>

#include "../../common/global_macros.h"
#include "../../common/logger.h"
#include "../../common/math/basic.h"
#include "../driver.h"
#include "buffer.h"
#include "command_buffer.h"
#include "swap_chain.h"
#include "texture.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class DriverGl : public Driver {
public:
    std::shared_ptr<Framebuffer> create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                    const std::shared_ptr<Texture> &texture,
                                                    const std::string &label) override;

    std::shared_ptr<Buffer> create_buffer(const BufferDescriptor &desc) override;

    std::shared_ptr<Texture> create_texture(const TextureDescriptor &desc) override;

    std::shared_ptr<CommandBuffer> create_command_buffer(const std::string &label) override;

    std::shared_ptr<RenderPass> create_render_pass(TextureFormat format,
                                                   AttachmentLoadOp load_op,
                                                   const std::string &label) override;

    std::shared_ptr<RenderPass> create_swap_chain_render_pass(TextureFormat format, AttachmentLoadOp load_op) override;

    std::shared_ptr<DescriptorSet> create_descriptor_set() override;

    std::shared_ptr<RenderPipeline> create_render_pipeline(
        const std::vector<char> &vert_source,
        const std::vector<char> &frag_source,
        const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
        BlendState blend_state,
        const std::shared_ptr<DescriptorSet> &descriptor_set,
        const std::shared_ptr<RenderPass> &render_pass,
        const std::string &label) override;

    std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::vector<char> &comp_source,
                                                             const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                             const std::string &label) override;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_DRIVER_GL_H
