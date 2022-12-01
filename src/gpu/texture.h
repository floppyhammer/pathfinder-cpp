#ifndef PATHFINDER_GPU_TEXTURE_H
#define PATHFINDER_GPU_TEXTURE_H

#include <utility>

#include "../common/global_macros.h"
#include "../common/math/vec2.h"
#include "data.h"

namespace Pathfinder {

class Texture {
public:
    Texture(Vec2I _size, TextureFormat _format, std::string _label)
        : size(_size), format(_format), label(std::move(_label)) {}

    inline Vec2I get_size() const {
        return size;
    }

    inline TextureFormat get_format() const {
        return format;
    }

protected:
    Vec2I size;

    TextureFormat format;

    /// Debug label.
    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_H
