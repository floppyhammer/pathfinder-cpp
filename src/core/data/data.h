#ifndef PATHFINDER_DATA_H
#define PATHFINDER_DATA_H

#include <vector>

#include "../../common/color.h"
#include "../../common/math/mat2x2.h"
#include "../../common/math/rect.h"
#include "../../common/math/transform2.h"
#include "../../common/math/vec2.h"
#include "../effects.h"

namespace Pathfinder {
// Tile size.
const int TILE_WIDTH = 16;
const int TILE_HEIGHT = 16;

// Curve type.
const unsigned int CURVE_IS_QUADRATIC = 0x80000000;
const unsigned int CURVE_IS_CUBIC = 0x40000000;

// Fill rule.
const int32_t TILE_CTRL_MASK_WINDING = 0x1;
const int32_t TILE_CTRL_MASK_EVEN_ODD = 0x2;
const int32_t TILE_CTRL_MASK_0_SHIFT = 0;

struct PushSegmentFlags {
    uint8_t value = 0x00;

    /// The bounds should be updated.
    static const uint8_t UPDATE_BOUNDS = 0x01;

    /// The "from" point of the segment.
    static const uint8_t INCLUDE_FROM_POINT = 0x02;

    PushSegmentFlags() = default;

    explicit PushSegmentFlags(uint8_t p_value) : value(p_value) {
    }
};

/// Flags that each point can have, indicating whether it's an on-curve or control point.
enum class PointFlag {
    ON_CURVE_POINT,

    /// This point is the first control point of a cubic Bézier curve or the only control point
    /// of a quadratic Bézier curve.
    CONTROL_POINT_0,

    /// This point is the second point of a cubic Bézier curve.
    CONTROL_POINT_1,
};

inline Rect<int> round_rect_out_to_tile_bounds(const Rect<float> &rect) {
    return (rect * Vec2<float>(1.0f / TILE_WIDTH, 1.0f / TILE_HEIGHT)).round_out();
}

/// Fill rule for a shape.
enum class FillRule {
    /// The nonzero rule: https://en.wikipedia.org/wiki/Nonzero-rule
    Winding,
    /// The even-odd rule: https://en.wikipedia.org/wiki/Even%E2%80%93odd_rule
    EvenOdd,
};

/// Fill style for a shape.
enum class FillStyle {
    Color,
    Gradient,
    Pattern,
};

/// The shape of the ends of the stroke.
enum class LineCap {
    /// The ends of lines are squared off at the endpoints.
    Butt,
    /// The ends of lines are squared off by adding a box with an equal width and half the height
    /// of the line's thickness.
    Square,
    /// The ends of lines are rounded.
    Round,
};

/// The shape used to join two line segments where they meet.
enum class LineJoin {
    /// Connected segments are joined by extending their outside edges to connect at a single
    /// point, with the effect of filling an additional lozenge-shaped area. The `f32` value
    /// specifies the miter limit ratio.
    Miter,
    /// Fills an additional triangular area between the common endpoint of connected segments and
    /// the separate outside rectangular corners of each segment.
    Bevel,
    /// Rounds off the corners of a shape by filling an additional sector of disc centered at the
    /// common endpoint of connected segments. The radius for these rounded corners is equal to the
    /// line width.
    Round,
};

enum class ArcDirection {
    /// Clockwise, starting from the +x axis.
    CW,
    /// Counterclockwise, starting from the +x axis.
    CCW,
};

enum class ColorCombineMode {
    None,
    SrcIn,
    DestIn,
};

inline int32_t color_combine_mode_to_composite_ctrl(ColorCombineMode color_combine_mode) {
    switch (color_combine_mode) {
        case ColorCombineMode::SrcIn:
            return COMBINER_CTRL_COLOR_COMBINE_SRC_IN;
        case ColorCombineMode::DestIn:
            return COMBINER_CTRL_COLOR_COMBINE_DEST_IN;
        case ColorCombineMode::None:
        default:
            return 0;
    }
}

struct TextureMetadataEntry {
    Transform2 color_0_transform;
    ColorCombineMode color_0_combine_mode = ColorCombineMode::SrcIn;
    ColorU base_color;
    Filter filter;
    BlendMode blend_mode = BlendMode::SrcIn;
};

struct Range {
    /// The lower bound of the range (inclusive).
    unsigned long long start = 0;

    /// The upper bound of the range (exclusive).
    unsigned long long end = 0;

    Range() = default;

    Range(unsigned long long p_start, unsigned long long p_end) : start(p_start), end(p_end){};
};
} // namespace Pathfinder

#endif // PATHFINDER_DATA_H
