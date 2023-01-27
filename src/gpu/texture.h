#ifndef PATHFINDER_GPU_TEXTURE_H
#define PATHFINDER_GPU_TEXTURE_H

#include <utility>

#include "../common/global_macros.h"
#include "../common/math/vec2.h"
#include "data.h"

namespace Pathfinder {

struct TextureDescriptor {
    Vec2I size;
    TextureFormat format;
    std::string label;
};

class Texture {
public:
    explicit Texture(TextureDescriptor _desc) : desc(std::move(_desc)) {}

    inline Vec2I get_size() const {
        return desc.size;
    }

    inline TextureFormat get_format() const {
        return desc.format;
    }

protected:
    TextureDescriptor desc;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_H
