#include "color.h"

namespace Pathfinder {

// ColorU
// --------------------
ColorU::ColorU(uint32_t color) {
    // Note that the order is reversed.
    a_ = color >> 24u;
    b_ = (color << 8u) >> 24u;
    g_ = (color << 16u) >> 24u;
    r_ = (color << 24u) >> 24u;
}

ColorU::ColorU(const ColorF& color) {
    // Note that the order is reversed.
    a_ = static_cast<uint8_t>(color.a_ * 255.f);
    b_ = static_cast<uint8_t>(color.b_ * 255.f);
    g_ = static_cast<uint8_t>(color.g_ * 255.f);
    r_ = static_cast<uint8_t>(color.r_ * 255.f);
}

ColorU::ColorU(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r_(r), g_(g), b_(b), a_(a) {}

ColorU::ColorU(uint8_t r, uint8_t g, uint8_t b) : r_(r), g_(g), b_(b), a_(255) {}

uint32_t ColorU::to_u32() const {
    uint32_t rgba = (r_ << 24u) + (g_ << 16u) + (b_ << 8u) + a_;

    return rgba;
}

ColorF ColorU::to_f32() const {
    float factor = 1.f / 255.f;

    return {static_cast<float>(r_) * factor,
            static_cast<float>(g_) * factor,
            static_cast<float>(b_) * factor,
            static_cast<float>(a_) * factor};
}

bool ColorU::is_opaque() const {
    return a_ != 0;
}

ColorU ColorU::lerp(const ColorU& other, float t) const {
    auto ret = ColorU(to_f32().lerp(other.to_f32(), t));
    return ret;
}

// --------------------

// ColorF
// --------------------
ColorF::ColorF(float r, float g, float b, float a) : r_(r), g_(g), b_(b), a_(a) {}

ColorF ColorF::lerp(const ColorF& other, float t) const {
    assert(t >= 0 && t <= 1);
    t = clamp(t, 0.0f, 1.0f);

    auto ret = ColorF{
        r_ + (other.r_ - r_) * t,
        g_ + (other.g_ - g_) * t,
        b_ + (other.b_ - b_) * t,
        a_ + (other.a_ - a_) * t,
    };

    return ret;
}
// --------------------

} // namespace Pathfinder
