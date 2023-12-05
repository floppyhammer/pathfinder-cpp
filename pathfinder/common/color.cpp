#include "color.h"

namespace Pathfinder {

// ColorU
// --------------------
ColorU::ColorU(uint32_t _color) {
    // Note that the order is reversed.
    a = _color >> 24u;
    b = (_color << 8u) >> 24u;
    g = (_color << 16u) >> 24u;
    r = (_color << 24u) >> 24u;
}

ColorU::ColorU(ColorF _color) {
    // Note that the order is reversed.
    a = static_cast<uint8_t>(_color.a * 255.f);
    b = static_cast<uint8_t>(_color.b * 255.f);
    g = static_cast<uint8_t>(_color.g * 255.f);
    r = static_cast<uint8_t>(_color.r * 255.f);
}

ColorU::ColorU(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {}

ColorU::ColorU(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b), a(255) {}

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
ColorF::ColorF(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}

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
