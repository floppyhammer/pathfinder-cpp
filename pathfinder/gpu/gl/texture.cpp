#include "texture.h"

#include <assert.h>

#include "../../config.h"
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

TextureGl::TextureGl(uint32_t external_gl_id, const TextureDescriptor& desc) : Texture(desc) {
    gl_id_ = external_gl_id;
    wrapped = true;
}

TextureGl::~TextureGl() {
    if (wrapped) {
        return;
    }
    glDeleteTextures(1, &gl_id_);

    if (pbo_id_ > 0) {
        glDeleteBuffers(1, &pbo_id_);
    }
}

uint32_t TextureGl::get_texture_id() const {
    return gl_id_;
}

uint32_t TextureGl::get_pbo_id() const {
    return pbo_id_;
}

void TextureGl::set_label(const std::string& label) {
    Texture::set_label(label);

    DebugMarker::label_texture(gl_id_, label);
}

void TextureGl::prepare_pbo() {
    if (pbo_id_ > 0) {
        return;
    }

    glGenBuffers(1, &pbo_id_);
    assert(pbo_id_ != 0);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_id_);

    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(desc_.byte_size()), nullptr, GL_STREAM_DRAW);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

} // namespace Pathfinder
