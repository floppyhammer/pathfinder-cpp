#pragma once

#include <memory>

#include "../gpu/device.h"
#include "../gpu/queue.h"

namespace Pathfinder {

/// Simply blits a texture to a framebuffer.
class Blit {
public:
    Blit(const std::shared_ptr<Device> &device, const std::shared_ptr<Queue> &queue, TextureFormat target_format);

    void set_texture(const std::shared_ptr<Texture> &new_texture);

    void update_uniform(const std::shared_ptr<CommandEncoder> &encoder);

    void draw(const std::shared_ptr<CommandEncoder> &encoder) const;

private:
    std::shared_ptr<Device> device_;

    std::shared_ptr<Queue> queue_;

    std::shared_ptr<Texture> texture_;

    std::shared_ptr<Buffer> uniform_buffer_;

    std::shared_ptr<RenderPipeline> pipeline_;

    std::shared_ptr<DescriptorSetLayout> descriptor_set_layout_;

    std::shared_ptr<DescriptorSet> descriptor_set_;

    std::shared_ptr<Sampler> sampler_;

    std::shared_ptr<Fence> fence_;
};

} // namespace Pathfinder
