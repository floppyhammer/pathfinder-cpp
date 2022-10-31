#ifndef PATHFINDER_DEMO_TEXTURE_RECT_H
#define PATHFINDER_DEMO_TEXTURE_RECT_H

#include <memory>

#include "../../src/gpu/command_buffer.h"
#include "../../src/gpu/descriptor_set.h"
#include "../../src/gpu/framebuffer.h"
#include "../../src/gpu/render_pipeline.h"
#include "../../src/gpu/texture.h"
#include "../../src/pathfinder.h"

/**
 * Simple class to blit a texture onto the screen.
 */
class TextureRect {
public:
    TextureRect(const std::shared_ptr<Pathfinder::Driver> &driver,
                const std::shared_ptr<Pathfinder::RenderPass> &render_pass,
                float width,
                float height);

    void set_texture(std::shared_ptr<Pathfinder::Texture> p_texture);

    void draw(const std::shared_ptr<Pathfinder::Driver> &driver,
              const std::shared_ptr<Pathfinder::CommandBuffer> &cmd_buffer,
              const Pathfinder::Vec2I &framebuffer_size);

    Pathfinder::Vec2F position;
    Pathfinder::Vec2F size;
    Pathfinder::Vec2F scale{1};

private:
    std::shared_ptr<Pathfinder::Texture> texture;

    std::shared_ptr<Pathfinder::RenderPipeline> pipeline;

    std::shared_ptr<Pathfinder::Buffer> vertex_buffer, uniform_buffer;

    std::shared_ptr<Pathfinder::DescriptorSet> descriptor_set;
};

#endif // PATHFINDER_DEMO_TEXTURE_RECT_H
