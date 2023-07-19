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

struct SamplerDescriptor {
    SamplerFilter mag_filter;
    SamplerFilter min_filter;
    SamplerAddressMode address_mode_u;
    SamplerAddressMode address_mode_v;

    inline bool operator==(const SamplerDescriptor& rhs) const {
        return mag_filter == rhs.mag_filter && min_filter == rhs.min_filter && address_mode_u == rhs.address_mode_u &&
               address_mode_v == rhs.address_mode_v;
    }
};

class Sampler {
public:
    Sampler(SamplerDescriptor _descriptor) : descriptor(_descriptor) {}

    SamplerDescriptor get_descriptor() const {
        return descriptor;
    }

protected:
    SamplerDescriptor descriptor;
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
