//
// Created by floppyhammer on 6/25/2021.
//

#include "viewport.h"

#include <cassert>

namespace Pathfinder {
    Viewport::Viewport(int p_width, int p_height) : width(p_width), height(p_height) {
        // Do nothing.
    }

    Viewport::Viewport(int p_width, int p_height, TextureFormat p_format, DataType p_type)
            : width(p_width), height(p_height) {
        // Create a texture.
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

    Viewport::~Viewport() {
        glDeleteFramebuffers(1, &framebuffer_id);
    }

    void Viewport::set_clear_color(const ColorF &color) {
        clear_color = color;
    }

    int Viewport::get_width() const {
        return width;
    }

    int Viewport::get_height() const {
        return height;
    }

    unsigned int Viewport::get_framebuffer_id() const {
        return framebuffer_id;
    }

    std::shared_ptr<Texture> Viewport::get_texture() {
        assert(texture != nullptr && "Tried to get texture from screen viewport!");
        return texture;
    }

    unsigned int Viewport::get_texture_id() const {
        if (texture) return texture->get_texture_id();

        assert("Tried to get texture id from screen viewport!");
        return 0;
    }

    ColorF Viewport::get_clear_color() const {
        return clear_color;
    }
}
