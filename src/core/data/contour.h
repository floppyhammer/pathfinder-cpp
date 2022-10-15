#ifndef PATHFINDER_CONTOUR_H
#define PATHFINDER_CONTOUR_H

#include <vector>

#include "../../common/math/rect.h"
#include "../../common/math/transform2.h"
#include "../../common/math/vec2.h"
#include "data.h"
#include "segment.h"

namespace Pathfinder {

/// An individual sub-path, consisting of a series of endpoints and/or control points. Contours can
/// be either open (first and last points disconnected) or closed (first point implicitly joined to
/// last point with a line).
/// Contours must start with the MoveTo command).
class Contour {
public:
    Contour() = default;

    std::vector<Vec2<float>> points;
    std::vector<PointFlag> flags;

    Rect<float> bounds = Rect<float>();

    /// If we should connect the end point to the start point.
    bool closed = false;

    bool is_empty() const;

    bool might_need_join(LineJoin join) const;

    Vec2<float> position_of_last(int index);

    void add_join(float distance, LineJoin join, Vec2<float> join_point, LineSegmentF next_tangent, float miter_limit);

    /// Adds a new on-curve point at the given position to this contour.
    void push_endpoint(const Vec2<float> &to);

    /// Adds a new quadratic Bézier curve to the given on-curve position and control point to this
    /// contour.
    void push_quadratic(const Vec2<float> &ctrl0, const Vec2<float> &to);

    /// Adds a new cubic Bézier curve to the given on-curve position and control points to this
    /// contour.
    void push_cubic(const Vec2<float> &ctrl0, const Vec2<float> &ctrl1, const Vec2<float> &to);

    /// Marks this contour as closed, which results in an implicit line from the end back to the
    /// starting point.
    void close();

    /// Push a point with a flag.
    void push_point(const Vec2<float> &point, PointFlag flag, bool update_bounds);

    /// Push a segment as points and flags.
    void push_segment(const Segment &segment, PushSegmentFlags p_flags);

    void push_arc_from_unit_chord(Transform2 &transform, LineSegmentF chord, ArcDirection direction);

    /// Use this function to keep bounds up to date when mutating contours.
    /// See `Outline::transform()` for an example of use.
    void update_bounds(Rect<float> &p_bounds) const;

    /**
     * Convert points in a contour to segments.
     * Segments, instead of points, are actually used at the stroking and tiling stages.
     * @note The CLOSED flag for a contour is also handled here.
     * @param force_closed When the contour is not closed and the fill is not transparent, we still need to
     * close the contour in order to properly tile it. That's why we have "force_closed" here.
     * Don't set it true when stroking.
     * @return Segments (e.g. lines, curves).
     */
    std::vector<Segment> get_segments(bool force_closed = false) const;

    void transform(const Transform2 &transform);
};

/// An iterator used to traverse segments efficiently in a contour.
class SegmentsIter {
public:
    SegmentsIter(const std::vector<Vec2<float>> &p_points, const std::vector<PointFlag> &p_flags, bool p_closed);

    /// Get next segment in the contour.
    Segment get_next(bool force_closed = false);

    bool is_at_start() const;
    bool is_at_end() const;

private:
    /// Contour data.
    const std::vector<Vec2<float>> &points;
    const std::vector<PointFlag> &flags;

    /// If the contour is closed.
    bool closed = false;

    /// Current point.
    int head = 0;

    /// If the contour has next segment.
    bool has_next = true;
};

} // namespace Pathfinder

#endif // PATHFINDER_CONTOUR_H
