#ifndef PATHFINDER_GPU_FRAMEBUFFER_H
#define PATHFINDER_GPU_FRAMEBUFFER_H

#include "texture.h"

#include <memory>

namespace Pathfinder {
    class Framebuffer {
    public:
        /// To screen viewport.
        Framebuffer(uint32_t p_width, uint32_t p_height)
                : width(p_width), height(p_height) {}

        /// To texture.
        Framebuffer(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type)
                : width(p_width), height(p_height) {}

        virtual std::shared_ptr<Texture> get_texture() = 0;

        virtual uint32_t get_unique_id() = 0;

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
        uint32_t width, height;
    };
}

#endif //PATHFINDER_GPU_FRAMEBUFFER_H
