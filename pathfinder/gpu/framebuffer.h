#pragma once

#include <memory>

#include "texture.h"

namespace Pathfinder {

/// Creation of a framebuffer is render pass dependent.
class Framebuffer {
public:
    virtual ~Framebuffer() = default;

    std::shared_ptr<Texture> get_texture() const {
        return texture_;
    }

    Vec2I get_size() const {
        if (texture_) {
            return texture_->get_size();
        }
        return {};
    }

protected:
    /// Render to screen or swap chain.
    Framebuffer() {}

    /// Render to a texture.
    explicit Framebuffer(const std::shared_ptr<Texture>& texture) : texture_(texture) {}

    std::shared_ptr<Texture> texture_;

    /// Debug label.
    std::string label_;
};

} // namespace Pathfinder
