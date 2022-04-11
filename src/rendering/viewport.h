//
// Created by floppyhammer on 6/1/2021.
//

#ifndef PATHFINDER_VIEWPORT_H
#define PATHFINDER_VIEWPORT_H

#include "texture.h"
#include "../common/color.h"

#include <memory>

namespace Pathfinder {
    // TODO: Refactor to RenderTarget.
    class Viewport {
    public:
        /// Screen viewport.
        Viewport(int p_width, int p_height);

        /// Offscreen viewport.
        Viewport(int p_width, int p_height, TextureFormat p_format, DataType p_type);

        ~Viewport();

        unsigned int get_framebuffer_id() const;

        std::shared_ptr<Texture> get_texture();

        unsigned int get_texture_id() const;

        /// Bind framebuffer.
        void use() const;

        /// Clear framebuffer.
        void clear() const;

        void set_clear_color(const ColorF &color);

        ColorF get_clear_color() const;

        int get_width() const;

        int get_height() const;

    private:
        int width, height;

        unsigned int framebuffer_id = 0;

        ColorF clear_color;

        // Only valid for offscreen viewports.
        std::shared_ptr<Texture> texture;
    };
}

#endif //PATHFINDER_VIEWPORT_H
