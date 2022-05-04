#ifndef PATHFINDER_SEGMENT_H
#define PATHFINDER_SEGMENT_H

#include "data.h"
#include "../../d3d9/data/line_segment.h"
#include "../../../common/math/transform2.h"

namespace Pathfinder {
    struct Path;

    /// A single line or Bézier curve segment, with explicit start and end points.
    struct Segment {
        // The start and end points of the curve.
        LineSegmentF baseline;

        // The control point(s).
        // If this is a line (which can be determined by examining the segment kind), this field is
        // ignored. If this is a quadratic Bézier curve, the start point of this line represents the
        // control point, and the endpoint of this line is ignored. Otherwise, if this is a cubic
        // Bézier curve, both the start and end points are used.
        LineSegmentF ctrl;

        // The type of segment this is: invalid, line, quadratic, or cubic Bézier curve.
        SegmentKind kind = SegmentKind::None;

        // Various flags that describe information about this segment in a path.
        SegmentFlags flags = SegmentFlags::NONE;

        Segment() = default;

        Segment(const LineSegmentF &p_baseline, const LineSegmentF &p_ctrl);

        Segment(const LineSegmentF &p_baseline, const LineSegmentF &p_ctrl, SegmentKind p_kind, SegmentFlags p_flags);

        inline static Segment line(const LineSegmentF &p_baseline) {
            return {p_baseline, LineSegmentF(), SegmentKind::Line, SegmentFlags::NONE};
        }

        inline static Segment quadratic(const LineSegmentF &p_baseline, const Vec2<float> &p_ctrl) {
            return {p_baseline, LineSegmentF(p_ctrl, p_ctrl), SegmentKind::Quadratic, SegmentFlags::NONE};
        }

        inline static Segment cubic(const LineSegmentF &p_baseline, const LineSegmentF &p_ctrl) {
            return {p_baseline, p_ctrl, SegmentKind::Cubic, SegmentFlags::NONE};
        }

        /**
         * Check if a cubic segment is flat enough to be treated as a line segment.
         * @param tol Calculation tolerance.
         * @return If it is true.
         */
        bool is_flat(float tol) const;

        /// Splits this segment.
        void split(float t, Segment &segment0, Segment &segment1) const;

        /// Splits this cubic Bézier curve into two at the given parametric t value,
        /// which will be clamped to the range 0.0 to 1.0.
        /// This uses de Casteljau subdivision.
        void split_cubic(float t, Segment &segment0, Segment &segment1) const;

        LineSegmentF as_line_segment() const;

        Segment to_cubic() const;

        Vec2<float> sample(float t) const;

        bool error_is_within_tolerance(const Segment &other, float distance) const;

        Segment offset_once(float distance) const;

        void add_to_path(float distance, LineJoin join, Vec2<float> join_point,
                            float join_miter_limit, Path &contour) const;

        void offset(float distance, LineJoin join, float join_miter_limit, Path &contour) const;

        /**
         * Change segment's orientation.
         * @return Reversed segment.
         */
        Segment reversed() const;

        /// Applies the given affine transform to this segment and returns it.
        Segment transform(Transform2 transform) const;

        /// Returns a cubic Bézier segment that approximates a quarter of an arc, centered on the +x axis.
        static Segment quarter_circle_arc() {
            auto p0 = Vec2<float>(std::sqrt(2.0f) * 0.5f);
            auto p1 = Vec2<float>(-std::sqrt(2.0f) / 6.0f + 4.0f / 3.0f, 7.0f * std::sqrt(2.0f) / 6.0f - 4.0f / 3.0f);
            auto flip = Vec2<float>(1.0, -1.0);

            auto p2 = p1 * flip;
            auto p3 = p0 * flip;

            return {LineSegmentF(p3, p0), LineSegmentF(p2, p1), SegmentKind::Cubic, SegmentFlags::NONE};
        }

        /// Approximates an unit-length arc with a cubic Bézier curve, given the cosine of the sweep
        /// angle.
        ///
        /// The maximum supported sweep angle is π/2 (i.e. 90°).
        static Segment arc_from_cos(float cos_sweep_angle) {
            const float EPSILON = 0.001;

            // Richard A. DeVeneza, "How to determine the control points of a Bézier curve that
            // approximates a small arc", 2004.
            //
            // https://www.tinaja.com/glib/bezcirc2.pdf
            if (cos_sweep_angle >= 1.0 - EPSILON) {
                return Segment::line(LineSegmentF(Vec2<float>(1.0, 0.0), Vec2<float>(1.0, 0.0)));
            }

            auto term = Vec2<float>(cos_sweep_angle, -cos_sweep_angle);

            auto signs_xy = Vec2<float>(1.0, -1.0);
            auto signs_zw = Vec2<float>(1.0, 1.0);

            auto p3 = ((term + 1.0f) * Vec2<float>(0.5f)).sqrt() * signs_xy;
            auto p0 = ((term + 1.0f) * Vec2<float>(0.5f)).sqrt() * signs_zw;

            auto p0x = p0.x;
            auto p0y = p0.y;

            auto p1x = 4.0f - p0x;
            auto p1y = (1.0f - p0x) * (3.0f - p0x) / p0y;

            auto p2 = Vec2<float>(p1x, -p1y) * (1.0f / 3.0f);
            auto p1 = Vec2<float>(p1x, p1y) * (1.0f / 3.0f);

            return Segment::cubic(LineSegmentF(p3, p0), LineSegmentF(p2, p1));
        }

        /// Returns true if this segment represents a straight line.
        inline bool is_line() const {
            return kind == SegmentKind::Line;
        }

        /// Returns true if this segment represents a quadratic Bézier curve.
        inline bool is_quadratic() const {
            return kind == SegmentKind::Quadratic;
        }

        /// Returns true if this segment represents a cubic Bézier curve.
        inline bool is_cubic() const {
            return kind == SegmentKind::Cubic;
        }

        /// FIXME: Make this more accurate for curves.
        float arc_length() const;
    };
}

#endif //PATHFINDER_SEGMENT_H
