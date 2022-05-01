#include "framebuffer.h"
#include "../framebuffer.h"


#include <cassert>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    FramebufferGl::FramebufferGl(uint32_t p_width, uint32_t p_height) : Framebuffer(p_width, p_height) {
        gl_framebuffer = 0;
    }

    FramebufferGl::FramebufferGl(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type)
            : Framebuffer(p_width, p_height) {
        // Create a color texture.
        texture = std::make_shared<TextureGl>(p_width, p_height, p_format, p_type);
        auto texture_gl = static_cast<TextureGl *>(texture.get());

        // Set up framebuffer.
        glGenFramebuffers(1, &gl_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gl_framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_gl->get_texture_id(), 0);

        // Always check whether our framebuffer is OK.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::error("GL_FRAMEBUFFER is not OK!");
        }
    }

    FramebufferGl::~FramebufferGl() {
        glDeleteFramebuffers(1, &gl_framebuffer);
    }

    uint32_t FramebufferGl::get_gl_framebuffer() const {
        return gl_framebuffer;
    }

    uint32_t FramebufferGl::get_unique_id() {
        return gl_framebuffer;
    }
}

#endif
