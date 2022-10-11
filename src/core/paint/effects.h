#ifndef PATHFINDER_EFFECTS_H
#define PATHFINDER_EFFECTS_H

#include "../../common/color.h"
#include "../data/line_segment.h"

namespace Pathfinder {

/// The axis a Gaussian blur is applied to.
enum BlurDirection {
    /// The horizontal axis.
    X,
    /// The vertical axis.
    Y,
};

/// Shaders applicable to patterns.
struct PatternFilter {
    enum class Type {
        Text,
        Blur,
    } type = Type::Blur;

    /// Performs postprocessing operations useful for monochrome text.
    struct Text {
        /// The foreground color of the text.
        ColorF fg_color;
        /// The background color of the text.
        ColorF bg_color;
    };

    /// A blur operation in one direction, either horizontal or vertical.
    ///
    /// To produce a full Gaussian blur, perform two successive blur operations, one in each
    /// direction.
    struct Blur {
        /// The axis of the blur: horizontal or vertical.
        BlurDirection direction{};
        /// Half the blur radius.
        float sigma = 0;
    };

    union {
        Text text;
        Blur blur;
    };

    /// We need this constructor to get union work.
    PatternFilter() {}
};

/// The shader that should be used when compositing this layer onto its destination.
struct PaintFilter {
    /// Filter type.
    enum class Type {
        None,
        RadialGradient,
        PatternFilter,
    } type = Type::None;

    /// Converts a linear gradient to a radial one.
    struct RadialGradient {
        /// The line that the circles lie along.
        LineSegmentF line;
        /// The radii of the circles at the two endpoints.
        Vec2<float> radii;
        /// The origin of the linearized gradient in the texture.
        Vec2<float> uv_origin;
    };

    RadialGradient gradient_filter; // For RadialGradient type.

    PatternFilter pattern_filter; // For PatternFilter type.
};

/// Blend modes that can be applied to individual paths.
enum class BlendMode {
    // Porter-Duff, supported by GPU blender.
    /// No regions are enabled.
    Clear,
    /// Only the source will be present.
    Copy,
    /// The source that overlaps the destination, replaces the destination.
    SrcIn,
    /// Source is placed, where it falls outside of the destination.
    SrcOut,
    /// Source is placed over the destination.
    SrcOver,
    /// Source which overlaps the destination, replaces the destination. Destination is placed
    /// elsewhere.
    SrcAtop,
    /// Destination which overlaps the source, replaces the source.
    DestIn,
    /// Destination is placed, where it falls outside of the source.
    DestOut,
    /// Destination is placed over the source.
    DestOver,
    /// Destination which overlaps the source replaces the source. Source is placed elsewhere.
    DestAtop,
    /// The non-overlapping regions of source and destination are combined.
    Xor,
    /// Display the sum of the source image and destination image. It is defined in the Porter-Duff
    /// paper as the plus operator.
    Lighter,

    // Others, unsupported by GPU blender.
    /// Selects the darker of the backdrop and source colors.
    Darken,
    /// Selects the lighter of the backdrop and source colors.
    Lighten,
    /// The source color is multiplied by the destination color and replaces the destination.
    Multiply,
    /// Multiplies the complements of the backdrop and source color values, then complements the
    /// result.
    Screen,
    /// Multiplies or screens the colors, depending on the source color value. The effect is
    /// similar to shining a harsh spotlight on the backdrop.
    HardLight,
    /// Multiplies or screens the colors, depending on the backdrop color value.
    Overlay,
    /// Brightens the backdrop color to reflect the source color.
    ColorDodge,
    /// Darkens the backdrop color to reflect the source color.
    ColorBurn,
    /// Darkens or lightens the colors, depending on the source color value. The effect is similar
    /// to shining a diffused spotlight on the backdrop.
    SoftLight,
    /// Subtracts the darker of the two constituent colors from the lighter color.
    Difference,
    /// Produces an effect similar to that of the Difference mode but lower in contrast.
    Exclusion,
    /// Creates a color with the hue of the source color and the saturation and luminosity of the
    /// backdrop color.
    Hue,
    /// Creates a color with the saturation of the source color and the hue and luminosity of the
    /// backdrop color.
    Saturation,
    /// Creates a color with the hue and saturation of the source color and the luminosity of the
    /// backdrop color.
    Color,
    /// Creates a color with the luminosity of the source color and the hue and saturation of the
    /// backdrop color. This produces an inverse effect to that of the Color mode.
    Luminosity,
};

const int32_t COMBINER_CTRL_COLOR_COMBINE_SRC_IN = 0x1;
const int32_t COMBINER_CTRL_COLOR_COMBINE_DEST_IN = 0x2;

const int32_t COMBINER_CTRL_COMPOSITE_NORMAL = 0x0;
const int32_t COMBINER_CTRL_COMPOSITE_MULTIPLY = 0x1;
const int32_t COMBINER_CTRL_COMPOSITE_SCREEN = 0x2;
const int32_t COMBINER_CTRL_COMPOSITE_OVERLAY = 0x3;
const int32_t COMBINER_CTRL_COMPOSITE_DARKEN = 0x4;
const int32_t COMBINER_CTRL_COMPOSITE_LIGHTEN = 0x5;
const int32_t COMBINER_CTRL_COMPOSITE_COLOR_DODGE = 0x6;
const int32_t COMBINER_CTRL_COMPOSITE_COLOR_BURN = 0x7;
const int32_t COMBINER_CTRL_COMPOSITE_HARD_LIGHT = 0x8;
const int32_t COMBINER_CTRL_COMPOSITE_SOFT_LIGHT = 0x9;
const int32_t COMBINER_CTRL_COMPOSITE_DIFFERENCE = 0xa;
const int32_t COMBINER_CTRL_COMPOSITE_EXCLUSION = 0xb;
const int32_t COMBINER_CTRL_COMPOSITE_HUE = 0xc;
const int32_t COMBINER_CTRL_COMPOSITE_SATURATION = 0xd;
const int32_t COMBINER_CTRL_COMPOSITE_COLOR = 0xe;
const int32_t COMBINER_CTRL_COMPOSITE_LUMINOSITY = 0xf;

inline int32_t blend_mode_to_composite_ctrl(BlendMode blend_mode) {
    switch (blend_mode) {
        case BlendMode::SrcOver:
        case BlendMode::SrcAtop:
        case BlendMode::DestOver:
        case BlendMode::DestOut:
        case BlendMode::Xor:
        case BlendMode::Lighter:
        case BlendMode::Clear:
        case BlendMode::Copy:
        case BlendMode::SrcIn:
        case BlendMode::SrcOut:
        case BlendMode::DestIn:
        case BlendMode::DestAtop:
            return COMBINER_CTRL_COMPOSITE_NORMAL;
        case BlendMode::Multiply:
            return COMBINER_CTRL_COMPOSITE_MULTIPLY;
        case BlendMode::Darken:
            return COMBINER_CTRL_COMPOSITE_DARKEN;
        case BlendMode::Lighten:
            return COMBINER_CTRL_COMPOSITE_LIGHTEN;
        case BlendMode::Screen:
            return COMBINER_CTRL_COMPOSITE_SCREEN;
        case BlendMode::Overlay:
            return COMBINER_CTRL_COMPOSITE_OVERLAY;
        case BlendMode::ColorDodge:
            return COMBINER_CTRL_COMPOSITE_COLOR_DODGE;
        case BlendMode::ColorBurn:
            return COMBINER_CTRL_COMPOSITE_COLOR_BURN;
        case BlendMode::HardLight:
            return COMBINER_CTRL_COMPOSITE_HARD_LIGHT;
        case BlendMode::SoftLight:
            return COMBINER_CTRL_COMPOSITE_SOFT_LIGHT;
        case BlendMode::Difference:
            return COMBINER_CTRL_COMPOSITE_DIFFERENCE;
        case BlendMode::Exclusion:
            return COMBINER_CTRL_COMPOSITE_EXCLUSION;
        case BlendMode::Hue:
            return COMBINER_CTRL_COMPOSITE_HUE;
        case BlendMode::Saturation:
            return COMBINER_CTRL_COMPOSITE_SATURATION;
        case BlendMode::Color:
            return COMBINER_CTRL_COMPOSITE_COLOR;
        case BlendMode::Luminosity:
            return COMBINER_CTRL_COMPOSITE_LUMINOSITY;
        default:
            return COMBINER_CTRL_COMPOSITE_NORMAL;
    }
}

/// True if this blend mode does not preserve destination areas outside the source.
inline bool is_blend_mode_destructive(BlendMode blend_mode) {
    switch (blend_mode) {
        case BlendMode::Clear:
        case BlendMode::Copy:
        case BlendMode::SrcIn:
        case BlendMode::DestIn:
        case BlendMode::SrcOut:
        case BlendMode::DestAtop: {
            return true;
        }
        case BlendMode::SrcOver:
        case BlendMode::DestOver:
        case BlendMode::DestOut:
        case BlendMode::SrcAtop:
        case BlendMode::Xor:
        case BlendMode::Lighter:
        case BlendMode::Lighten:
        case BlendMode::Darken:
        case BlendMode::Multiply:
        case BlendMode::Screen:
        case BlendMode::HardLight:
        case BlendMode::Overlay:
        case BlendMode::ColorDodge:
        case BlendMode::ColorBurn:
        case BlendMode::SoftLight:
        case BlendMode::Difference:
        case BlendMode::Exclusion:
        case BlendMode::Hue:
        case BlendMode::Saturation:
        case BlendMode::Color:
        case BlendMode::Luminosity:
        default: {
            return false;
        }
    }
}

struct TextureLocation {
    /// Which texture.
    uint32_t page{};
    /// Region in the texture.
    Rect<uint32_t> rect{};
};

struct TextureSamplingFlags {
    uint8_t value = 0;

    static const uint8_t REPEAT_U = 0x01;
    static const uint8_t REPEAT_V = 0x02;
    static const uint8_t NEAREST_MIN = 0x04;
    static const uint8_t NEAREST_MAG = 0x08;
};

} // namespace Pathfinder

#endif // PATHFINDER_EFFECTS_H
