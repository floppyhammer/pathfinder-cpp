//
// Created by floppyhammer on 6/7/2021.
//

#include "texture.h"

#include "../common/global_macros.h"

namespace Pathfinder {
    Texture::Texture(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type)
            : width(p_width), height(p_height), format(p_format), type(p_type) {
        // Allocate a texture.
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        // We need to use glTexStorage2D() in order to access the texture via image2D in compute shaders.
#ifdef PATHFINDER_USE_D3D11
        // Allocate space.
        glTexStorage2D(GL_TEXTURE_2D, 1, static_cast<GLint>(format), width, height);
#else
        // Allocate space and (optional) upload data.
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format),
                     width, height, 0,
                     static_cast<GLint>(PixelDataFormat::RGBA),
                     static_cast<GLenum>(type), nullptr);
#endif

        // Set texture wrapping parameters.
        // Noteable: It has to be CLAMP_TO_EDGE. Artifacts will show for both REPEAT and MIRRORED_REPEAT.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Set texture filtering parameters.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &texture_id);
    }

    uint32_t Texture::get_width() const {
        return width;
    }

    uint32_t Texture::get_height() const {
        return height;
    }

    Vec2<uint32_t> Texture::get_size() const {
        return {width, height};
    }

    uint32_t Texture::get_texture_id() const {
        return texture_id;
    }

    TextureFormat Texture::get_format() const {
        return format;
    }

    DataType Texture::get_pixel_type() const {
        return type;
    }
}
