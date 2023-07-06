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

    size_t byte_size() const {
        return size.area() * get_pixel_size(format);
    }

    inline bool operator==(const TextureDescriptor& b) const {
        return size == b.size && format == b.format;
    }
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

    // Sometimes, we need to update label for a texture as we reuse it for another purpose.
    virtual void set_label(const std::string& _label) {
        label = _label;
    }

protected:
    TextureDescriptor desc;

    bool resource_ownership = true;

    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_H
