#ifndef PATHFINDER_DEMO_TEXTURE_RECT_H
#define PATHFINDER_DEMO_TEXTURE_RECT_H

#include <pathfinder/prelude.h>

#include <memory>

/**
 * Simple class to blit a texture to screen.
 */
class TextureRect {
public:
    TextureRect(const std::shared_ptr<Pathfinder::Device> &_device,
                const std::shared_ptr<Pathfinder::Queue> &_queue,
                const std::shared_ptr<Pathfinder::RenderPass> &render_pass);

    void set_texture(const std::shared_ptr<Pathfinder::Texture> &new_texture);

    void draw(const std::shared_ptr<Pathfinder::CommandEncoder> &encoder, const Pathfinder::Vec2I &framebuffer_size);

private:
    Pathfinder::Vec2F position;
    Pathfinder::Vec2F size;
    Pathfinder::Vec2F scale{1};

    std::shared_ptr<Pathfinder::Device> device;

    std::shared_ptr<Pathfinder::Queue> queue;

    std::shared_ptr<Pathfinder::Texture> texture;

    std::shared_ptr<Pathfinder::RenderPipeline> pipeline;

    std::shared_ptr<Pathfinder::Buffer> vertex_buffer, uniform_buffer;

    std::shared_ptr<Pathfinder::DescriptorSet> descriptor_set;

    std::shared_ptr<Pathfinder::Sampler> sampler;
};

#endif // PATHFINDER_DEMO_TEXTURE_RECT_H
