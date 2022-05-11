#ifndef PATHFINDER_STROKE_H
#define PATHFINDER_STROKE_H

#include <vector>

#include "data/path.h"
#include "data/segment.h"
#include "data/shape.h"
#include "../../common/math/transform2.h"

//! Utilities for converting path strokes to fills.

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

    /// Path stroke to fill.
    struct PathStrokeToFill {
    public:
        PathStrokeToFill(Path p_input, float p_radius, LineJoin p_join, float p_join_miter_limit);

        /// Scale the input path up, forming an outer path.
        void offset_forward();

        /// Scale the input path down, forming an inner path.
        void offset_backward();

        Path input;
        Path output;

    private:
        float radius;
        LineJoin join;
        float join_miter_limit = 10; // Only used when line join is miter.
    };

    /// Strokes a shape with a stroke style to produce a new shape.
    struct ShapeStrokeToFill {
    public:
        ShapeStrokeToFill(const Shape &p_input, StrokeStyle p_style);

        /// Performs the stroke operation.
        void offset();

        /// Returns the stroked shape. This should be called after `offset()`.
        Shape into_shape() const;

    private:
        const Shape &input;
        Shape output{};
        StrokeStyle style;

        void push_stroked_path(std::vector<Path> &new_contours, PathStrokeToFill stroker, bool closed) const;

        void add_cap(Path &contour) const;
    };
}

#endif //PATHFINDER_STROKE_H
