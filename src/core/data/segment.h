#ifndef PATHFINDER_SEGMENT_H
#define PATHFINDER_SEGMENT_H

#include "../../common/math/transform2.h"
#include "data.h"
#include "line_segment.h"

namespace Pathfinder {

class Contour;

/// A single line or Bézier curve segment, with explicit start and end points.
class Segment {
public:
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

    // Various flags that describe information about this segment in a contour.
    SegmentFlags flags = SegmentFlags::NONE;

    Segment() = default;

    Segment(const LineSegmentF &_baseline, const LineSegmentF &_ctrl);

    Segment(const LineSegmentF &_baseline, const LineSegmentF &_ctrl, SegmentKind _kind, SegmentFlags _flags);

    inline static Segment line(const LineSegmentF &baseline) {
        return {baseline, LineSegmentF(), SegmentKind::Line, SegmentFlags::NONE};
    }

    inline static Segment quadratic(const LineSegmentF &baseline, const Vec2F &ctrl) {
        return {baseline, LineSegmentF(ctrl, ctrl), SegmentKind::Quadratic, SegmentFlags::NONE};
    }

    inline static Segment cubic(const LineSegmentF &baseline, const LineSegmentF &ctrl) {
        return {baseline, ctrl, SegmentKind::Cubic, SegmentFlags::NONE};
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

    Vec2F sample(float t) const;

    bool error_is_within_tolerance(const Segment &other, float distance) const;

    Segment offset_once(float distance) const;

    void add_to_contour(float distance,
                        LineJoin join,
                        Vec2F join_point,
                        float join_miter_limit,
                        Contour &contour) const;

    /**
     * Offset segment by a distance.
     * @param distance Distance to offset in normal direction. Negative is outward and positive is inward.
     * @param join Join type.
     * @param join_miter_limit Only for miter join.
     * @param contour Target contour to add new segment.
     */
    void offset(float distance, LineJoin join, float join_miter_limit, Contour &contour) const;

    /**
     * Change segment's orientation.
     * @return Reversed segment.
     */
    Segment reversed() const;

    /// Applies the given affine transform to this segment and returns it.
    Segment transform(Transform2 transform) const;

    /// Returns a cubic Bézier segment that approximates a quarter of an arc, centered on the +x axis.
    static Segment quarter_circle_arc();

    /// Approximates an unit-length arc with a cubic Bézier curve, given the cosine of the sweep
    /// angle.
    ///
    /// The maximum supported sweep angle is π/2 (i.e. 90°).
    static Segment arc_from_cos(float cos_sweep_angle);

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

    bool is_valid() const;
};

} // namespace Pathfinder

#endif // PATHFINDER_SEGMENT_H
