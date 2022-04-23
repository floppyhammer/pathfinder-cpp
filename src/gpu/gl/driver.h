#ifndef PATHFINDER_GPU_DRIVER_GL_H
#define PATHFINDER_GPU_DRIVER_GL_H

#include "swap_chain.h"
#include "buffer.h"
#include "texture.h"
#include "command_buffer.h"
#include "../driver.h"
#include "../../common/math/basic.h"
#include "../../common/global_macros.h"
#include "../../common/logger.h"

#include <vector>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class DriverGl : public Driver {
    public:
        std::shared_ptr<SwapChain> create_swap_chain(uint32_t p_width, uint32_t p_height) override;

        std::shared_ptr<Framebuffer> create_framebuffer(uint32_t p_width,
                                                        uint32_t p_height,
                                                        TextureFormat p_format,
                                                        DataType p_type,
                                                        const std::shared_ptr<RenderPass> &render_pass) override;

        std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size) override;

        std::shared_ptr<Texture> create_texture(uint32_t p_width,
                                                uint32_t p_height,
                                                TextureFormat p_format,
                                                DataType p_type) override;

        std::shared_ptr<CommandBuffer> create_command_buffer() override;

        std::shared_ptr<RenderPass> create_render_pass(TextureFormat format) override;

        std::shared_ptr<RenderPipeline> create_render_pipeline(const std::vector<char> &vert_source,
                                                               const std::vector<char> &frag_source,
                                                               const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                                                               ColorBlendState blend_state,
                                                               const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                               const std::shared_ptr<RenderPass> &render_pass) override;

        std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::vector<char> &comp_source,
                                                                 const std::shared_ptr<DescriptorSet> &descriptor_set) override;
    };
}

#endif

#endif //PATHFINDER_GPU_DRIVER_GL_H
