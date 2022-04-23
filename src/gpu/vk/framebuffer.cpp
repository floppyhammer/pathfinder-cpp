#include "framebuffer.h"

#include <cassert>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    FramebufferVk::FramebufferVk(int p_width, int p_height) : Framebuffer(p_width, p_height) {
        // Do nothing.
    }

    FramebufferVk::FramebufferVk(int p_width, int p_height, TextureFormat p_format, DataType p_type)
            : Framebuffer(p_width, p_height) {
        // Create a color texture.
        texture = std::make_shared<TextureGl>(p_width, p_height, p_format, p_type);

        // Set up framebuffer.
        glGenFramebuffers(1, &framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->get_texture_id(), 0);

        // Always check whether our framebuffer is OK.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::error("GL_FRAMEBUFFER is not OK!");
        }
    }

    FramebufferVk::~FramebufferVk() {
        glDeleteFramebuffers(1, &framebuffer_id);
    }

    VkFramebuffer FramebufferVk::get_framebuffer_id() const {
        return framebuffer_id;
    }

    std::shared_ptr<Texture> FramebufferVk::get_texture() {
        assert(texture != nullptr && "Tried to get texture from screen viewport!");
        return texture;
    }

    uint32_t FramebufferVk::get_unique_id() {
        return reinterpret_cast<uint32_t>(&framebuffer_id);;
    }
}

#endif
