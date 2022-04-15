//
// Created by floppyhammer on 6/7/2021.
//

#ifndef PATHFINDER_TEXTURE_RECT_H
#define PATHFINDER_TEXTURE_RECT_H

#include "control.h"
#include "../../../rendering/gl/framebuffer.h"
#include "../../../rendering/gl/texture.h"
#include "../../../rendering/gl/render_pipeline.h"
#include "../../../rendering/gl/descriptor_set.h"
#include "../../../rendering/gl/command_buffer.h"

#include <memory>

namespace Pathfinder {
    class TextureRect : public Control {
    public:
        TextureRect(float viewport_width, float viewport_height);

        void set_texture(std::shared_ptr<Texture> p_texture);

        [[nodiscard]] std::shared_ptr<Texture> get_texture() const;

        void draw(const std::shared_ptr<Pathfinder::CommandBuffer>& cmd_buffer, const std::shared_ptr<Framebuffer>& render_target);

    private:
        std::shared_ptr<Texture> texture;

        std::shared_ptr<RenderPipeline> pipeline;

        std::shared_ptr<Buffer> vertex_buffer, uniform_buffer;

        std::shared_ptr<DescriptorSet> descriptor_set;
    };
}

#endif //PATHFINDER_TEXTURE_RECT_H
