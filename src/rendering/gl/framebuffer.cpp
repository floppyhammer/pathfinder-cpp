//
// Created by floppyhammer on 6/25/2021.
//

#include "framebuffer.h"

#include <cassert>

namespace Pathfinder {
    Framebuffer::Framebuffer(int p_width, int p_height) : width(p_width), height(p_height) {
        // Do nothing.
    }

    Framebuffer::Framebuffer(int p_width, int p_height, TextureFormat p_format, DataType p_type)
            : width(p_width), height(p_height) {
        // Create a color texture.
        texture = std::make_shared<Texture>(p_width, p_height, p_format, p_type);

        // Set up framebuffer.
        glGenFramebuffers(1, &framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->get_texture_id(), 0);

        // Always check whether our framebuffer is OK.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::error("GL_FRAMEBUFFER is not OK!");
        }
    }

    Framebuffer::~Framebuffer() {
        glDeleteFramebuffers(1, &framebuffer_id);
    }

    uint32_t Framebuffer::get_width() const {
        return width;
    }

    uint32_t Framebuffer::get_height() const {
        return height;
    }

    Vec2<uint32_t> Framebuffer::get_size() const {
        return {width, height};
    }

    uint32_t Framebuffer::get_framebuffer_id() const {
        return framebuffer_id;
    }

    std::shared_ptr<Texture> Framebuffer::get_texture() {
        assert(texture != nullptr && "Tried to get texture from screen viewport!");
        return texture;
    }

    uint32_t Framebuffer::get_texture_id() const {
        if (texture) return texture->get_texture_id();

        assert("Tried to get texture id from screen viewport!");
        return 0;
    }
}
