#include "texture.h"

#include "../../common/global_macros.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
TextureGl::TextureGl(uint32_t p_width, uint32_t p_height, TextureFormat p_format)
    : Texture(p_width, p_height, p_format) {
    // We can deduce the pixel data type by the texture format.
    DataType type = texture_format_to_data_type(p_format);

    // Generate a texture.
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Allocate space.
    // We need to use glTexStorage2D() in order to access the texture via image2D in compute shaders.
    #ifdef PATHFINDER_USE_D3D11
    glTexStorage2D(GL_TEXTURE_2D, 1, to_gl_texture_format(p_format), width, height);
    #else
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 to_gl_texture_format(p_format),
                 width,
                 height,
                 0,
                 GL_RGBA,
                 to_gl_data_type(type),
                 nullptr);
    #endif

    // Set texture sampler.
    // Set wrapping parameters.
    // Noteable: It has to be CLAMP_TO_EDGE. Artifacts will show for both REPEAT and MIRRORED_REPEAT.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set filtering parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

TextureGl::~TextureGl() {
    glDeleteTextures(1, &texture_id);
}

uint32_t TextureGl::get_texture_id() const {
    return texture_id;
}
} // namespace Pathfinder

#endif
