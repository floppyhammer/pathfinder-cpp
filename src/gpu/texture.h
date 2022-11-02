#ifndef PATHFINDER_GPU_TEXTURE_H
#define PATHFINDER_GPU_TEXTURE_H

#include "../common/global_macros.h"
#include "../common/math/vec2.h"
#include "data.h"

namespace Pathfinder {

class Texture {
public:
    Texture(int32_t _width, int32_t _height, TextureFormat _format) : width(_width), height(_height), format(_format) {}

    inline int32_t get_width() const {
        return width;
    }

    inline int32_t get_height() const {
        return height;
    }

    inline Vec2I get_size() const {
        return {width, height};
    }

    inline TextureFormat get_format() const {
        return format;
    }

protected:
    int32_t width;
    int32_t height;

    TextureFormat format;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_H
