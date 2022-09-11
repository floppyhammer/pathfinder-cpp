#ifndef PATHFINDER_GPU_TEXTURE_H
#define PATHFINDER_GPU_TEXTURE_H

#include "data.h"
#include "../common/math/vec2.h"
#include "../common/global_macros.h"

namespace Pathfinder {
    class Texture {
    public:
        Texture(uint32_t p_width, uint32_t p_height, TextureFormat p_format)
                : width(p_width), height(p_height), format(p_format) {}

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

        inline TextureLayout get_layout() const {
            return layout;
        }

        inline void set_layout(TextureLayout new_layout) {
            layout = new_layout;
        }

    protected:
        uint32_t width;
        uint32_t height;

        TextureFormat format;

        TextureLayout layout = TextureLayout::UNDEFINED;
    };
}

#endif //PATHFINDER_GPU_TEXTURE_H
