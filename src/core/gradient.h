#ifndef PATHFINDER_GRADIENT_H
#define PATHFINDER_GRADIENT_H

#include <vector>

#include "../common/color.h"
#include "../common/math/transform2.h"
#include "data/line_segment.h"

//! Gradient effects that paths can be filled with.
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
    Vec2<float> radii;

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
    std::shared_ptr<LineSegmentF> linear;

    /// A radial gradient that radiates outward from a line connecting two circles (or from one
    /// circle).
    std::shared_ptr<GradientRadial> radial;
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
struct Gradient {
    /// Information specific to the type of gradient (linear or radial).
    GradientGeometry geometry;

    std::vector<ColorStop> stops;

    /// What should be rendered upon reaching the end of the color stops.
    GradientWrap wrap = GradientWrap::Repeat;

    /// Creates a new linear gradient with the given line.
    ///
    /// The line is in scene coordinates, not relative to the bounding box of the current path.
    static Gradient linear(const LineSegmentF &line) {
        Gradient gradient;
        gradient.geometry.type = GradientGeometry::Type::Linear;
        gradient.geometry.linear = std::make_shared<LineSegmentF>(line);
        gradient.wrap = GradientWrap::Clamp;

        return gradient;
    }

    /// Creates a new radial gradient from a line connecting the centers of two circles with the
    /// given radii, or a point at the center of one circle.
    ///
    /// To create a radial gradient with a single circle (the common case), pass a `Vector2F`
    /// representing the center of the circle for `line`; otherwise, to create a radial gradient
    /// with two circles, pass a `LineSegment2F`. To start the gradient at the center of the
    /// circle, pass zero for the first radius.
    static Gradient radial(const LineSegmentF &line, const Vec2<float> &radii) {
        Gradient gradient;
        gradient.geometry.type = GradientGeometry::Type::Radial;
        gradient.geometry.radial = std::make_shared<GradientRadial>();
        gradient.geometry.radial->line = line;
        gradient.geometry.radial->radii = radii;
        gradient.wrap = GradientWrap::Clamp;

        return gradient;
    }

    /// Adds a new color stop to the radial gradient.
    inline void add(const ColorStop stop) {
        stops.push_back(stop);
    }

    /// A convenience method equivalent to
    /// `gradient.add_color_stop(ColorStop::new(color, offset))`.
    inline void add_color_stop(const ColorU color, const float offset) {
        add(ColorStop{offset, color});
    }

    /// Returns true if all colors of all stops in this gradient are opaque.
    inline bool is_opaque() {
        bool opaque = true;
        for (auto &stop : stops) {
            opaque &= stop.color.is_opaque();
        }
        return opaque;
    }
};
} // namespace Pathfinder

#endif // PATHFINDER_GRADIENT_H
