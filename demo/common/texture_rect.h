#ifndef PATHFINDER_DEMO_TEXTURE_RECT_H
#define PATHFINDER_DEMO_TEXTURE_RECT_H

#include <memory>

#include "../../src/gpu/command_buffer.h"
#include "../../src/gpu/descriptor_set.h"
#include "../../src/gpu/framebuffer.h"
#include "../../src/gpu/render_pipeline.h"
#include "../../src/gpu/texture.h"
#include "pathfinder.h"

using namespace Pathfinder;

/**
 * Simple class to blit a texture to screen.
 */
class TextureRect {
public:
    TextureRect(const std::shared_ptr<Device> &_device, const std::shared_ptr<RenderPass> &render_pass);

    void set_texture(const std::shared_ptr<Texture> &new_texture);

    void draw(const std::shared_ptr<CommandBuffer> &cmd_buffer, const Vec2I &framebuffer_size);

private:
    Vec2F position;
    Vec2F size;
    Vec2F scale{1};

    std::shared_ptr<Device> device;

    std::shared_ptr<Texture> texture;

    std::shared_ptr<RenderPipeline> pipeline;

    std::shared_ptr<Buffer> vertex_buffer, uniform_buffer;

    std::shared_ptr<DescriptorSet> descriptor_set;
};

#endif // PATHFINDER_DEMO_TEXTURE_RECT_H
