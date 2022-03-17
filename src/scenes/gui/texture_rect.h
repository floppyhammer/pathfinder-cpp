//
// Created by chy on 6/7/2021.
//

#ifndef PATHFINDER_TEXTURE_RECT_H
#define PATHFINDER_TEXTURE_RECT_H

#include "control.h"
#include "../../rendering/viewport.h"
#include "../../rendering/texture.h"
#include "../../rendering/raster_program.h"

#include <memory>

namespace Pathfinder {
    class TextureRect : public Control {
    public:
        TextureRect(float viewport_width, float viewport_height);

        ~TextureRect();

        void set_texture(std::shared_ptr<Texture> p_texture);

        void attach_shader(std::shared_ptr<RasterProgram> p_shader);

        std::shared_ptr<Texture> get_texture() const;

        void draw();

    private:
        std::shared_ptr<Texture> texture;

        std::shared_ptr<RasterProgram> shader;

        unsigned int vao = 0;
        unsigned int vbo = 0;
    };
}

#endif //PATHFINDER_TEXTURE_RECT_H
