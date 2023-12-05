#ifndef PATHFINDER_STROKE_H
#define PATHFINDER_STROKE_H

//! Utilities for converting contour strokes to fills.

#include <vector>

#include "../common/math/transform2.h"
#include "data/contour.h"
#include "data/path.h"
#include "data/segment.h"

namespace Pathfinder {

/// How an outline should be stroked.
struct StrokeStyle {
    /// The width of the stroke in scene units.
    float line_width = 1.0f;

    /// The shape of the ends of the stroke.
    LineCap line_cap = LineCap::Butt;

    /// The shape used to join two line segments where they meet.
    LineJoin line_join = LineJoin::Miter;

    /// Only for LineJoin::Miter.
    float miter_limit = 10.0f;
};

/// Contour stroke to fill.
struct ContourStrokeToFill {
    Contour input;
    Contour output;
    float radius;
    LineJoin join;
    float join_miter_limit = 10; // Only used when line join is miter.

    ContourStrokeToFill(Contour _input, float _radius, LineJoin _join, float _join_miter_limit);

    /// Scale the input contour up, forming an outer contour.
    void offset_forward();

    /// Scale the input contour down, forming an inner contour.
    void offset_backward();
};

/// Strokes an outline with a stroke style to produce a new shape.
struct OutlineStrokeToFill {
    const Outline &input;
    Outline output{};
    StrokeStyle style;

    OutlineStrokeToFill(const Outline &_input, StrokeStyle _style);

    /// Performs the stroke operation.
    void offset();

    /// Returns the resulting stroked outline. This should be called after `offset()`.
    Outline into_outline() const;

    void push_stroked_contour(std::vector<Contour> &new_contours, ContourStrokeToFill stroker, bool closed) const;

    void add_cap(Contour &contour) const;
};

} // namespace Pathfinder

#endif // PATHFINDER_STROKE_H
