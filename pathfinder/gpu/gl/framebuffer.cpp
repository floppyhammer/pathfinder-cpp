#include "framebuffer.h"

#include <cassert>
#include <utility>

#include "../framebuffer.h"
#include "debug_marker.h"

namespace Pathfinder {

FramebufferGl::FramebufferGl(Vec2I size) : Framebuffer(size) {
    gl_framebuffer_ = 0;
    label_ = "Screen framebuffer";
}

FramebufferGl::FramebufferGl(const std::shared_ptr<Texture> &texture) : Framebuffer(texture) {
    auto texture_gl = static_cast<TextureGl *>(texture.get());

    // Set up framebuffer.
    glGenFramebuffers(1, &gl_framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, gl_framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_gl->get_texture_id(), 0);

    // Check whether the framebuffer is OK.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Logger::error("Framebuffer is not OK!", "FramebufferGl");
    }

    gl_check_error("create_framebuffer");
}

FramebufferGl::~FramebufferGl() {
    if (gl_framebuffer_ != 0) {
        glDeleteFramebuffers(1, &gl_framebuffer_);
    }
}

uint32_t FramebufferGl::get_gl_framebuffer() const {
    return gl_framebuffer_;
}

unsigned long long FramebufferGl::get_unique_id() {
    return gl_framebuffer_;
}

void FramebufferGl::set_label(const std::string &_label) {
    Framebuffer::set_label(_label);

    DebugMarker::label_framebuffer(gl_framebuffer_, label_);
}

} // namespace Pathfinder
