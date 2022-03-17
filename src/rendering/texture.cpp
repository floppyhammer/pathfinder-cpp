//
// Created by chy on 6/7/2021.
//

#include "texture.h"

#include "../common/global_macros.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>

namespace Pathfinder {
    Texture::Texture(int p_width, int p_height, TextureFormat p_format, DataType p_type, const void *p_data)
            : width(p_width), height(p_height), format(p_format), type(p_type) {
        // Allocate a texture.
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        // We need to use glTexStorage2D() in order to access the texture via image2D in compute shaders.
#ifdef PATHFINDER_USE_D3D11
        // Allocate space.
        glTexStorage2D(GL_TEXTURE_2D, 1, static_cast<GLint>(format), width, height);

        // Upload data.
        if (p_data) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                            static_cast<GLint>(PixelDataFormat::RGBA),
                            static_cast<GLenum>(type), p_data);
        }
#else
        // Allocate space and (optional) upload data.
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format),
                     width, height, 0,
                     static_cast<GLint>(PixelDataFormat::RGBA),
                     static_cast<GLenum>(type), p_data);
#endif

        // Set texture wrapping parameters.
        // Noteable: It has to be CLAMP_TO_EDGE. Artifacts will show for both REPEAT and MIRRORED_REPEAT.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Set texture filtering parameters.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        DeviceGl::check_error("Texture::Texture()");
    }

    Texture::~Texture() {
        glDeleteTextures(1, &texture_id);
        DeviceGl::check_error("Texture::~Texture()");
    }

    unsigned int Texture::get_width() const {
        return width;
    }

    unsigned int Texture::get_height() const {
        return height;
    }

    unsigned int Texture::get_texture_id() const {
        return texture_id;
    }

    void Texture::update_region(const Rect<int> &p_rect, const void *p_data) const {
        glBindTexture(GL_TEXTURE_2D, texture_id);

        glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                p_rect.left,
                p_rect.top,
                p_rect.width(),
                p_rect.height(),
                static_cast<GLint>(PixelDataFormat::RGBA),
                static_cast<GLenum>(type),
                p_data
        );

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    TextureFormat Texture::get_format() const {
        return format;
    }

    DataType Texture::get_pixel_type() const {
        return type;
    }
}
