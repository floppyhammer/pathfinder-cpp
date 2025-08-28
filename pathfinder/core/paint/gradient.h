#ifndef PATHFINDER_GRADIENT_H
#define PATHFINDER_GRADIENT_H

//! Gradient effects that paths can be filled with.

#include <vector>

#include "../../common/math/transform2.h"
#include "effects.h"
#include "texture_allocator.h"

namespace Pathfinder {

/// A color in a gradient.
/// Points in a gradient between two stops interpolate linearly between the stops.
struct ColorStop {
    /// The offset of the color stop, between 0.0 and 1.0 inclusive. The value 0.0 represents the
    /// start of the gradient, and 1.0 represents the end.
    float offset = 0;

    /// The color of the gradient stop.
    ColorU color;
};

struct GradientRadial {
    /// The line that connects the centers of the two circles. For single-circle radial
    /// gradients (the common case), this line has zero length, with start point and endpoint
    /// both at the circle's center point.
    ///
    /// This is in scene coordinates, not relative to the bounding box of the path.
    LineSegmentF line;

    /// The radii of the two circles. The first value may be zero to start the gradient at the
    /// center of the circle.
    Vec2F radii;

    /// Transform from radial gradient space into screen space.
    ///
    /// Like `gradientTransform` in SVG. Note that this is the inverse of Cairo's gradient
    /// transform.
    Transform2 transform;
};

/// The type of gradient: linear or radial.
struct GradientGeometry {
    enum class Type {
        Linear,
        Radial,
    } type = Type::Linear;

    /// A linear gradient that follows a line.
    ///
    /// The line is in scene coordinates, not relative to the bounding box of the path.
    LineSegmentF linear;

    /// A radial gradient that radiates outward from a line connecting two circles (or from one
    /// circle).
    GradientRadial radial;

    void apply_transform(const Transform2 &transform) {
        if (type == Type::Linear) {
            linear = linear.apply_transform(transform);
        } else {
            radial.transform = transform * radial.transform;
        }
    }
};

/// What should be rendered outside the color stops.
enum class GradientWrap {
    /// The area before the gradient is filled with the color of the first stop, and the area after
    /// the gradient is filled with the color of the last stop.
    Clamp,

    /// The gradient repeats indefinitely.
    Repeat,
};

/// A gradient, either linear or radial.
class Gradient {
public:
    /// Information specific to the type of gradient (linear or radial).
    GradientGeometry geometry;

    /// What should be rendered upon reaching the end of the color stops.
    GradientWrap wrap = GradientWrap::Clamp;

    /// Creates a new linear gradient with the given line.
    ///
    /// The line is in scene coordinates, not relative to the bounding box of the current path.
    static Gradient linear(const LineSegmentF &line) {
        Gradient gradient;
        gradient.geometry.type = GradientGeometry::Type::Linear;
        gradient.geometry.linear = line;

        return gradient;
    }

    /// Creates a new radial gradient from a line connecting the centers of two circles with the
    /// given radii, or a point at the center of one circle.
    ///
    /// To create a radial gradient with a single circle (the common case), pass a `Vector2F`
    /// representing the center of the circle for `line`; otherwise, to create a radial gradient
    /// with two circles, pass a `LineSegment2F`. To start the gradient at the center of the
    /// circle, pass zero for the first radius.
    static Gradient radial(const LineSegmentF &line, const Vec2F &radii) {
        Gradient gradient;
        gradient.geometry.type = GradientGeometry::Type::Radial;
        gradient.geometry.radial.line = line;
        gradient.geometry.radial.radii = radii;

        return gradient;
    }

    /// Adds a new color stop by order of offset to the gradient.
    void add(const ColorStop &_stop);

    /// A convenience method equivalent to
    /// `gradient.add(ColorStop::new(color, offset))`.
    void add_color_stop(const ColorU &color, float offset);

    /// Returns the value of the gradient at offset `t`, which will be clamped between 0.0 and 1.0.
    ColorU sample(float t) const;

    /// Returns true if all colors of all stops in this gradient are opaque.
    bool is_opaque();

    // For being used as ordered key.
    bool operator<(const Gradient &rhs) const {
        if (wrap == rhs.wrap) {
            if (geometry.type == rhs.geometry.type) {
                if (geometry.type == GradientGeometry::Type::Linear) {
                    return geometry.linear < rhs.geometry.linear;
                } else {
                    bool res = geometry.radial.line < rhs.geometry.radial.line;
                    res = res && geometry.radial.radii < rhs.geometry.radial.radii;
                    res = res && geometry.radial.transform < rhs.geometry.radial.transform;
                    return res;
                }
            } else {
                return geometry.type < rhs.geometry.type;
            }
        } else {
            return wrap < rhs.wrap;
        }
    }

private:
    /// Color stops.
    std::vector<ColorStop> stops;
};

/// The size of a gradient tile.
constexpr uint32_t GRADIENT_TILE_LENGTH = 256;

/// Color texture data for gradient filter,
/// each row of which stores a 1D gradient line.
struct GradientTile {
    std::vector<ColorU> texels;
    uint32_t page;
    uint32_t next_index;
};

class GradientTileBuilder {
public:
    std::vector<GradientTile> tiles;

    /// Handles allocation via PaintTextureManager (no GPU resources involved here).
    TextureLocation allocate(const Gradient &gradient,
                             TextureAllocator &allocator,
                             std::vector<TextureLocation> transient_paint_locations);
};

} // namespace Pathfinder

#endif // PATHFINDER_GRADIENT_H
