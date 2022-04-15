//
// Created by floppyhammer on 6/1/2021.
//

#ifndef PATHFINDER_FRAMEBUFFER_H
#define PATHFINDER_FRAMEBUFFER_H

#include "texture.h"

#include <memory>

namespace Pathfinder {
    class Framebuffer {
    public:
        /// To screen viewport.
        Framebuffer(int p_width, int p_height);

        /// To texture.
        Framebuffer(int p_width, int p_height, TextureFormat p_format, DataType p_type);

        ~Framebuffer();

        uint32_t get_framebuffer_id() const;

        std::shared_ptr<Texture> get_texture();

        uint32_t get_texture_id() const;

        uint32_t get_width() const;

        uint32_t get_height() const;

        Vec2<uint32_t> get_size() const;

    private:
        uint32_t width, height;

        uint32_t framebuffer_id = 0;

        // Only valid when drawing to a texture.
        std::shared_ptr<Texture> texture;
    };
}

#endif //PATHFINDER_FRAMEBUFFER_H
