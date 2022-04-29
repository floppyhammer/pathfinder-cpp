//
// Created by floppyhammer on 6/7/2021.
//

#ifndef PATHFINDER_TEXTURE_RECT_H
#define PATHFINDER_TEXTURE_RECT_H

#include "control.h"
#include "../../../gpu/gl/framebuffer.h"
#include "../../../gpu/gl/texture.h"
#include "../../../gpu/gl/render_pipeline.h"
#include "../../../gpu/descriptor_set.h"
#include "../../../gpu/gl/command_buffer.h"

#include <memory>

namespace Pathfinder {
    class TextureRect : public Control {
    public:
        TextureRect(const std::shared_ptr<Driver> &driver,
                    const std::shared_ptr<RenderPass> &render_pass,
                    uint32_t viewport_width,
                    uint32_t viewport_height);

        void set_texture(std::shared_ptr<Texture> p_texture);

        [[nodiscard]] std::shared_ptr<Texture> get_texture() const;

        void draw(const std::shared_ptr<Driver> &driver,
                  const std::shared_ptr<Pathfinder::CommandBuffer>& cmd_buffer,
                  const Vec2<uint32_t> &framebuffer_size);

    private:
        std::shared_ptr<Texture> texture;

        bool ignore_texture_size = false;

        std::shared_ptr<RenderPipeline> pipeline;

        std::shared_ptr<Buffer> vertex_buffer, uniform_buffer;

        std::shared_ptr<DescriptorSet> descriptor_set;
    };
}

#endif //PATHFINDER_TEXTURE_RECT_H
