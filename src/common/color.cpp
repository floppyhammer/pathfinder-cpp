#include "color.h"

namespace Pathfinder {

// ColorU
// --------------------
ColorU::ColorU(uint32_t p_color) {
    // Note that the order is reversed.
    a = p_color >> 24u;
    b = (p_color << 8u) >> 24u;
    g = (p_color << 16u) >> 24u;
    r = (p_color << 24u) >> 24u;
}

ColorU::ColorU(ColorF p_color) {
    // Note that the order is reversed.
    a = static_cast<uint8_t>(p_color.a * 255.f);
    b = static_cast<uint8_t>(p_color.b * 255.f);
    g = static_cast<uint8_t>(p_color.g * 255.f);
    r = static_cast<uint8_t>(p_color.r * 255.f);
}

ColorU::ColorU(uint8_t p_r, uint8_t p_g, uint8_t p_b, uint8_t p_a) : r(p_r), g(p_g), b(p_b), a(p_a) {}

ColorU::ColorU(uint8_t p_r, uint8_t p_g, uint8_t p_b) : r(p_r), g(p_g), b(p_b), a(255) {}

uint32_t ColorU::to_u32() const {
    uint32_t rgba = (r << 24u) + (g << 16u) + (b << 8u) + a;

    return rgba;
}

ColorF ColorU::to_f32() const {
    float factor = 1.f / 255.f;

    return {static_cast<float>(r) * factor,
            static_cast<float>(g) * factor,
            static_cast<float>(b) * factor,
            static_cast<float>(a) * factor};
}

bool ColorU::is_opaque() const {
    return a != 0;
}
// --------------------

// ColorF
// --------------------
ColorF::ColorF(float p_r, float p_g, float p_b, float p_a) : r(p_r), g(p_g), b(p_b), a(p_a) {}

ColorF ColorF::lerp(const ColorF& other, float t) const {
    return {
        r + (other.r - r) * t,
        g + (other.g - g) * t,
        b + (other.b - b) * t,
        a + (other.a - a) * t,
    };
}
// --------------------

} // namespace Pathfinder
