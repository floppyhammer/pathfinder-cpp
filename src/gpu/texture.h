#ifndef PATHFINDER_GPU_TEXTURE_H
#define PATHFINDER_GPU_TEXTURE_H

#include "../common/global_macros.h"
#include "../common/math/vec2.h"
#include "data.h"

namespace Pathfinder {

class Texture {
public:
    Texture(Vec2I _size, TextureFormat _format) : size(_size), format(_format) {}

    inline int32_t get_width() const {
        return size.x;
    }

    inline int32_t get_height() const {
        return size.y;
    }

    inline Vec2I get_size() const {
        return size;
    }

    inline TextureFormat get_format() const {
        return format;
    }

public:
    /// For debugging.
    std::string name;

protected:
    Vec2I size;

    TextureFormat format;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_H
