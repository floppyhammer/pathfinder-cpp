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
        std::shared_ptr<Framebuffer> create_framebuffer(uint32_t p_width,
                                                        uint32_t p_height,
                                                        TextureFormat p_format,
                                                        const std::shared_ptr<RenderPass> &render_pass) override;

        std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size, MemoryProperty property) override;

        std::shared_ptr<Texture> create_texture(uint32_t p_width,
                                                uint32_t p_height,
                                                TextureFormat p_format) override;

        std::shared_ptr<CommandBuffer> create_command_buffer(bool one_time) override;

        std::shared_ptr<RenderPass> create_render_pass(TextureFormat format, ImageLayout final_layout) override;

        std::shared_ptr<DescriptorSet> create_descriptor_set() override;

        std::shared_ptr<RenderPipeline> create_render_pipeline(const std::vector<char> &vert_source,
                                                               const std::vector<char> &frag_source,
                                                               const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                                                               ColorBlendState blend_state,
                                                               Vec2<uint32_t> viewport_size,
                                                               const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                               const std::shared_ptr<RenderPass> &render_pass) override;

        std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::vector<char> &comp_source,
                                                                 const std::shared_ptr<DescriptorSet> &descriptor_set) override;
    };
}

#endif

#endif //PATHFINDER_GPU_DRIVER_GL_H
