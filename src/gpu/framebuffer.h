#ifndef PATHFINDER_GPU_FRAMEBUFFER_H
#define PATHFINDER_GPU_FRAMEBUFFER_H

#include <cassert>
#include <memory>
#include <utility>

#include "texture.h"

namespace Pathfinder {

/**
 * Creation of a framebuffer is render pass dependent.
 */
class Framebuffer {
public:
    /// Render to screen or swap chain.
    explicit Framebuffer(Vec2I _size) : size(_size) {}

    /// Render to a texture.
    explicit Framebuffer(const std::shared_ptr<Texture>& _texture, std::string _label)
        : texture(_texture), size(_texture->get_size()), label(std::move(_label)) {}

    inline std::shared_ptr<Texture> get_texture() const {
        return texture;
    }

    /// Get the unique resource ID for the framebuffer, which is only used for hashing.
    virtual unsigned long long get_unique_id() = 0;

    inline Vec2I get_size() const {
        if (texture) {
            return texture->get_size();
        }
        return size;
    }

protected:
    Vec2I size;

    std::shared_ptr<Texture> texture;

    /// Debug label.
    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_FRAMEBUFFER_H
