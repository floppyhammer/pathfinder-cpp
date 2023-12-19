#ifndef PATHFINDER_GPU_FRAMEBUFFER_H
#define PATHFINDER_GPU_FRAMEBUFFER_H

#include <cassert>
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

    /// Get the unique resource ID for the framebuffer.
    virtual unsigned long long get_unique_id() = 0;

    Vec2I get_size() const {
        if (texture_) {
            return texture_->get_size();
        }
        return size_;
    }

    // Sometimes, we need to update label for a framebuffer as we reuse it for another purpose.
    virtual void set_label(const std::string& label) {
        label_ = label;
    }

protected:
    /// Render to screen or swap chain.
    explicit Framebuffer(const Vec2I& size) : size_(size) {}

    /// Render to a texture.
    explicit Framebuffer(const std::shared_ptr<Texture>& texture) : size_(texture->get_size()), texture_(texture) {}

    Vec2I size_;

    std::shared_ptr<Texture> texture_;

    /// Debug label.
    std::string label_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_FRAMEBUFFER_H
