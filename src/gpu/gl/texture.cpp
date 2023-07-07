#include "texture.h"

#include <utility>

#include "../../common/global_macros.h"
#include "debug_marker.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

TextureGl::TextureGl(const TextureDescriptor& _desc) : Texture(_desc) {
    // Generate a texture.
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Allocate space.
    // We need to use glTexStorage2D() in order to access the texture via image2D in compute shaders.
    #ifdef PATHFINDER_USE_D3D11
    glTexStorage2D(GL_TEXTURE_2D, 1, to_gl_texture_format(_desc.format), _desc.size.x, _desc.size.y);
    #else
    // We can deduce the pixel data type by the texture format.
    DataType type = texture_format_to_data_type(_desc.format);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 to_gl_texture_format(_desc.format),
                 _desc.size.x,
                 _desc.size.y,
                 0,
                 GL_RGBA,
                 to_gl_data_type(type),
                 nullptr);
    #endif

    // Set texture sampler.
    // Set wrapping parameters.
    // Note: It has to be CLAMP_TO_EDGE. Artifacts will show for both REPEAT and MIRRORED_REPEAT.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set filtering parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error("create_texture");

    DebugMarker::label_texture(texture_id, label);
}

TextureGl::~TextureGl() {
    glDeleteTextures(1, &texture_id);
}

uint32_t TextureGl::get_texture_id() const {
    return texture_id;
}

void TextureGl::set_label(const std::string& _label) {
    Texture::set_label(_label);

    DebugMarker::label_texture(texture_id, _label);
}

} // namespace Pathfinder

#endif
