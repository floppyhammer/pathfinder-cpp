#ifndef PATHFINDER_COLOR_H
#define PATHFINDER_COLOR_H

#include <cstdint>

#include "color.h"

namespace Pathfinder {
/// Color(0~1, 0~1, 0~1, 0~1).
struct ColorF {
    float r_ = 0;
    float g_ = 0;
    float b_ = 0;
    float a_ = 0;

    ColorF() = default;

    ColorF(float r, float g, float b, float a);

    ColorF lerp(const ColorF& other, float t) const;

    static ColorF red() {
        return {1, 0, 0, 1};
    }

    static ColorF green() {
        return {0, 1, 0, 1};
    }

    static ColorF blue() {
        return {0, 0, 1, 1};
    }

    static ColorF white() {
        return {1, 1, 1, 1};
    }

    static ColorF black() {
        return {0, 0, 0, 1};
    }

    ColorF operator*(ColorF other) const {
        return {r_ * other.r_, g_ * other.g_, b_ * other.b_, a_ * other.a_};
    }
};

/// Color(0~255, 0~255, 0~255, 0~255).
struct ColorU {
    uint8_t r_ = 0;
    uint8_t g_ = 0;
    uint8_t b_ = 0;
    uint8_t a_ = 0;

    ColorU() = default;

    explicit ColorU(uint32_t color);

    explicit ColorU(ColorF color);

    ColorU(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    ColorU(uint8_t r, uint8_t g, uint8_t b);

    uint32_t to_u32() const;

    ColorF to_f32() const;

    /// Check for transparency.
    bool is_opaque() const;

    bool operator<(const ColorU& rhs) const {
        return to_u32() < rhs.to_u32();
    }

    static ColorU red() {
        return {255, 0, 0, 255};
    }

    static ColorU green() {
        return {0, 255, 0, 255};
    }

    static ColorU blue() {
        return {0, 0, 255, 255};
    }

    static ColorU white() {
        return {255, 255, 255, 255};
    }

    static ColorU black() {
        return {0, 0, 0, 255};
    }

    static ColorU transparent_black() {
        return {0, 0, 0, 0};
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_COLOR_H
