#ifndef PATHFINDER_DEMO_TEXTURE_RECT_H
#define PATHFINDER_DEMO_TEXTURE_RECT_H

#include <pathfinder/prelude.h>

#include <memory>

/// Simply blits a texture to a framebuffer.
class Blit {
public:
    Blit(const std::shared_ptr<Pathfinder::Device> &device,
         const std::shared_ptr<Pathfinder::Queue> &queue,
         Pathfinder::TextureFormat target_format);

    void set_texture(const std::shared_ptr<Pathfinder::Texture> &new_texture);

    void draw(const std::shared_ptr<Pathfinder::CommandEncoder> &encoder);

private:
    std::shared_ptr<Pathfinder::Device> device_;

    std::shared_ptr<Pathfinder::Queue> queue_;

    std::shared_ptr<Pathfinder::Texture> texture_;

    std::shared_ptr<Pathfinder::RenderPipeline> pipeline_;

    std::shared_ptr<Pathfinder::Buffer> vertex_buffer_;

    std::shared_ptr<Pathfinder::DescriptorSet> descriptor_set_;

    std::shared_ptr<Pathfinder::Sampler> sampler_;
};

#endif // PATHFINDER_DEMO_TEXTURE_RECT_H
