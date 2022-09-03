#ifndef PATHFINDER_DEMO_TEXTURE_RECT_H
#define PATHFINDER_DEMO_TEXTURE_RECT_H

#include "../../src/gpu/framebuffer.h"
#include "../../src/gpu/texture.h"
#include "../../src/gpu/render_pipeline.h"
#include "../../src/gpu/descriptor_set.h"
#include "../../src/gpu/command_buffer.h"
#include "pathfinder.h"

#include <memory>

/**
 * Simple class to blit a texture onto the screen.
 */
class TextureRect {
public:
    TextureRect(const std::shared_ptr<Pathfinder::Driver> &driver,
                const std::shared_ptr<Pathfinder::RenderPass> &render_pass,
                uint32_t width,
                uint32_t height);

    void set_texture(std::shared_ptr<Pathfinder::Texture> p_texture);

    void draw(const std::shared_ptr<Pathfinder::Driver> &driver,
              const std::shared_ptr<Pathfinder::CommandBuffer> &cmd_buffer,
              const Pathfinder::Vec2<uint32_t> &framebuffer_size);

private:
    Pathfinder::Vec2<float> position{0};
    Pathfinder::Vec2<float> size;
    Pathfinder::Vec2<float> scale{1};

    std::shared_ptr<Pathfinder::Texture> texture;

    std::shared_ptr<Pathfinder::RenderPipeline> pipeline;

    std::shared_ptr<Pathfinder::Buffer> vertex_buffer, uniform_buffer;

    std::shared_ptr<Pathfinder::DescriptorSet> descriptor_set;
};

#endif //PATHFINDER_DEMO_TEXTURE_RECT_H
