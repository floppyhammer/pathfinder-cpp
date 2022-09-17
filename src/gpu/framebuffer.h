#ifndef PATHFINDER_GPU_FRAMEBUFFER_H
#define PATHFINDER_GPU_FRAMEBUFFER_H

#include "texture.h"

#include <memory>
#include <cassert>

namespace Pathfinder {
    /**
     * Creation of a framebuffer is render pass dependent.
     */
    class Framebuffer {
    public:
        /// Render to screen or swap chain.
        Framebuffer(uint32_t p_width, uint32_t p_height)
                : width(p_width), height(p_height) {}

        /// Render to a texture.
        explicit Framebuffer(const std::shared_ptr<Texture> &p_texture)
                : texture(p_texture), width(p_texture->get_width()), height(p_texture->get_height()) {}

        inline std::shared_ptr<Texture> get_texture() {
            assert(texture != nullptr && "No valid texture set to the framebuffer!");
            return texture;
        }

        /// Get the unique resource ID for the framebuffer, which is only used for hashing.
        virtual unsigned long long get_unique_id() = 0;

        inline uint32_t get_width() const {
            return width;
        }

        inline uint32_t get_height() const {
            return height;
        }

        inline Vec2<uint32_t> get_size() const {
            return {width, height};
        }

    protected:
        uint32_t width;
        uint32_t height;

        std::shared_ptr<Texture> texture;
    };
}

#endif //PATHFINDER_GPU_FRAMEBUFFER_H
