#ifndef PATHFINDER_GPU_TEXTURE_H
#define PATHFINDER_GPU_TEXTURE_H

#include "../common/math/vec2.h"
#include "base.h"

namespace Pathfinder {

struct TextureDescriptor {
    Vec2I size;
    TextureFormat format;

    size_t byte_size() const {
        return size.area() * get_pixel_size(format);
    }

    bool operator==(const TextureDescriptor& b) const {
        return size == b.size && format == b.format;
    }
};

struct SamplerDescriptor {
    SamplerFilter mag_filter;
    SamplerFilter min_filter;
    SamplerAddressMode address_mode_u;
    SamplerAddressMode address_mode_v;

    bool operator==(const SamplerDescriptor& rhs) const {
        return mag_filter == rhs.mag_filter && min_filter == rhs.min_filter && address_mode_u == rhs.address_mode_u &&
               address_mode_v == rhs.address_mode_v;
    }
};

class Sampler {
    friend class DeviceGl;

public:
    virtual ~Sampler() = default;

    SamplerDescriptor get_descriptor() const {
        return descriptor_;
    }

protected:
    explicit Sampler(const SamplerDescriptor& descriptor) : descriptor_(descriptor) {}

    SamplerDescriptor descriptor_;
};

class Texture {
public:
    virtual ~Texture() = default;

    Vec2I get_size() const {
        return desc_.size;
    }

    TextureFormat get_format() const {
        return desc_.format;
    }

    // Sometimes, we need to update label for a texture as we reuse it for another purpose.
    virtual void set_label(const std::string& label) {
        label_ = label;
    }

    std::string get_label() const {
        return label_;
    }

protected:
    explicit Texture(TextureDescriptor desc) : desc_(desc) {}

    TextureDescriptor desc_;

    bool resource_ownership_ = true;

    std::string label_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_H
