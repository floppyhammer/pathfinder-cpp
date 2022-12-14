#ifndef PATHFINDER_DEMO_TEXTURE_RECT_H
#define PATHFINDER_DEMO_TEXTURE_RECT_H

#include <memory>

#include "../../src/gpu/command_buffer.h"
#include "../../src/gpu/descriptor_set.h"
#include "../../src/gpu/framebuffer.h"
#include "../../src/gpu/render_pipeline.h"
#include "../../src/gpu/texture.h"
#include "../../src/pathfinder.h"

using namespace Pathfinder;

/**
 * Simple class to blit a texture onto the screen.
 */
class TextureRect {
public:
    TextureRect(const std::shared_ptr<Driver> &driver,
                const std::shared_ptr<RenderPass> &render_pass,
                const Vec2F &_size);

    void set_texture(const std::shared_ptr<Texture> &_texture);

    void draw(const std::shared_ptr<Driver> &driver,
              const std::shared_ptr<CommandBuffer> &cmd_buffer,
              const Vec2I &framebuffer_size);

    Vec2F position;
    Vec2F size;
    Vec2F scale{1};

private:
    std::shared_ptr<Texture> texture;

    std::shared_ptr<RenderPipeline> pipeline;

    std::shared_ptr<Buffer> vertex_buffer, uniform_buffer;

    std::shared_ptr<DescriptorSet> descriptor_set;
};

#endif // PATHFINDER_DEMO_TEXTURE_RECT_H
