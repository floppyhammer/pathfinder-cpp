#include "texture.h"

#include "../../common/global_macros.h"
#include "debug_marker.h"

namespace Pathfinder {

TextureGl::TextureGl(const TextureDescriptor& desc) : Texture(desc) {
    // Generate a texture.
    glGenTextures(1, &gl_id_);
    glBindTexture(GL_TEXTURE_2D, gl_id_);

// Allocate space.
// We need to use glTexStorage2D() in order to access the texture via image2D in compute shaders.
#ifdef PATHFINDER_ENABLE_D3D11
    glTexStorage2D(GL_TEXTURE_2D, 1, to_gl_texture_format(desc.format), desc.size.x, desc.size.y);
#else
    // We can deduce the pixel data type by the texture format.
    DataType type = texture_format_to_data_type(desc.format);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 to_gl_texture_format(desc.format),
                 desc.size.x,
                 desc.size.y,
                 0,
                 GL_RGBA,
                 to_gl_data_type(type),
                 nullptr);
#endif

    glBindTexture(GL_TEXTURE_2D, 0);

    gl_check_error("create_texture");

    DebugMarker::label_texture(gl_id_, label_);
}

TextureGl::~TextureGl() {
    glDeleteTextures(1, &gl_id_);
}

uint32_t TextureGl::get_texture_id() const {
    return gl_id_;
}

void TextureGl::set_label(const std::string& label) {
    Texture::set_label(label);

    DebugMarker::label_texture(gl_id_, label);
}

} // namespace Pathfinder
