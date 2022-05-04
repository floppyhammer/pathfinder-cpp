#ifndef PATHFINDER_COLOR_H
#define PATHFINDER_COLOR_H

#include "color.h"

#include <cstdint>

namespace Pathfinder {
    /// Color(0~1, 0~1, 0~1, 0~1).
    struct ColorF {
        float r = 0;
        float g = 0;
        float b = 0;
        float a = 0;

        ColorF() = default;

        ColorF(float p_r, float p_g, float p_b, float p_a);

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
    };

    /// Color(0~255, 0~255, 0~255, 0~255).
    struct ColorU {
        uint8_t r = 0u;
        uint8_t g = 0u;
        uint8_t b = 0u;
        uint8_t a = 0u;

        ColorU() = default;

        explicit ColorU(uint32_t p_color);

        explicit ColorU(ColorF p_color);

        ColorU(uint8_t p_r, uint8_t p_g, uint8_t p_b, uint8_t p_a);

        uint32_t to_u32() const;

        ColorF to_f32() const;

        bool is_transparent() const;

        bool is_opaque() const;

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
    };
}

#endif //PATHFINDER_COLOR_H
