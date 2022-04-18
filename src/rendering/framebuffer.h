#ifndef PATHFINDER_HAL_FRAMEBUFFER_H
#define PATHFINDER_HAL_FRAMEBUFFER_H

#include "texture.h"

#include <memory>

namespace Pathfinder {
    class Framebuffer {
    public:
        /// To screen viewport.
        Framebuffer(int p_width, int p_height)
                : width(p_width), height(p_height) {}

        /// To texture.
        Framebuffer(int p_width, int p_height, TextureFormat p_format, DataType p_type)
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

#endif //PATHFINDER_HAL_FRAMEBUFFER_H
