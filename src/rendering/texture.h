#ifndef PATHFINDER_HAL_TEXTURE_H
#define PATHFINDER_HAL_TEXTURE_H

#include "data.h"
#include "../common/math/vec2.h"
#include "../common/global_macros.h"

namespace Pathfinder {
    class Texture {
    public:
        Texture(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type)
                : width(p_width), height(p_height), format(p_format), type(p_type) {}

        inline uint32_t get_width() const {
            return width;
        }

        inline uint32_t get_height() const {
            return height;
        }

        inline Vec2<uint32_t> get_size() const {
            return {width, height};
        }

        inline TextureFormat get_format() const {
            return format;
        }

        inline DataType get_pixel_type() const {
            return type;
        }

    protected:
        /// Size.
        uint32_t width = 0;
        uint32_t height = 0;

        /// Pixel data type (GPU).
        TextureFormat format;

        /// Pixel data type (CPU).
        DataType type = DataType::UNSIGNED_BYTE;
    };
}

#endif //PATHFINDER_HAL_TEXTURE_H
