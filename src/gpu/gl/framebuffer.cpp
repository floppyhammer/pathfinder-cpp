#include "framebuffer.h"

#include <cassert>
#include <utility>

#include "../framebuffer.h"
#include "validation.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

FramebufferGl::FramebufferGl(Vec2I _size) : Framebuffer(_size) {
    gl_framebuffer = 0;
    label = "Screen framebuffer";
}

FramebufferGl::FramebufferGl(const std::shared_ptr<Texture> &_texture, std::string _label)
    : Framebuffer(_texture, std::move(_label)) {
    auto texture_gl = static_cast<TextureGl *>(texture.get());

    // Set up framebuffer.
    glGenFramebuffers(1, &gl_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gl_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_gl->get_texture_id(), 0);

    // Always check whether our framebuffer is OK.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Logger::error(
            "Framebuffer is not OK!"
            "FramebufferGl");
    }

    DebugMarker::label_framebuffer(gl_framebuffer, label);
}

FramebufferGl::~FramebufferGl() {
    glDeleteFramebuffers(1, &gl_framebuffer);
}

uint32_t FramebufferGl::get_gl_framebuffer() const {
    return gl_framebuffer;
}

unsigned long long FramebufferGl::get_unique_id() {
    return gl_framebuffer;
}

} // namespace Pathfinder

#endif
