#include "contour.h"

namespace Pathfinder {

Vec2F Contour::position_of_last(int index) {
    return points[points.size() - index];
}

void Contour::push_segment(const Segment &segment, PushSegmentFlags _flags) {
    // Not a valid segment.
    if (segment.kind == SegmentKind::None) {
        return;
    }

    auto update_bounds = (_flags.value & PushSegmentFlags::UPDATE_BOUNDS) > 0x00;

    // Push the first on-curve point.
    push_point(segment.baseline.from(), PointFlag::ON_CURVE_POINT, update_bounds);

    // Not a line.
    if (!segment.is_line()) {
        // Push the first control point.
        push_point(segment.ctrl.from(), PointFlag::CONTROL_POINT_0, update_bounds);

        // Not a quadratic curve.
        if (!segment.is_quadratic()) {
            // Push the second control point.
            push_point(segment.ctrl.to(), PointFlag::CONTROL_POINT_1, update_bounds);
        }
    }

    // Push the second on-curve point.
    push_point(segment.baseline.to(), PointFlag::ON_CURVE_POINT, update_bounds);
}

void Contour::update_bounds(RectF &_bounds) const {
    // The bounds to union is not valid.
    if (!_bounds.is_valid()) {
        _bounds = bounds;
    }

    _bounds = _bounds.union_rect(bounds);
}

std::vector<Segment> Contour::get_segments(bool force_closed) const {
    std::vector<Segment> segments;

    auto segments_iter = SegmentsIter(points, flags, closed);

    // Traverse curve/line segments.
    while (!segments_iter.is_at_end()) {
        auto segment = segments_iter.get_next(force_closed);
        if (segment.kind == SegmentKind::None) break;
        segments.push_back(segment);
    }

    return segments;
}

void Contour::close() {
    closed = true;
}

void Contour::push_point(const Vec2F &point, PointFlag flag, bool update_bounds) {
    if (update_bounds) {
        auto first = points.empty();
        union_rect(bounds, point, first);
    }

    points.push_back(point);
    flags.push_back(flag);
}

void Contour::push_endpoint(const Vec2F &to) {
    push_point(to, PointFlag::ON_CURVE_POINT, true);
}

void Contour::push_quadratic(const Vec2F &ctrl0, const Vec2F &to) {
    push_point(ctrl0, PointFlag::CONTROL_POINT_0, true);
    push_point(to, PointFlag::ON_CURVE_POINT, true);
}

void Contour::push_cubic(const Vec2F &ctrl0, const Vec2F &ctrl1, const Vec2F &to) {
    push_point(ctrl0, PointFlag::CONTROL_POINT_0, true);
    push_point(ctrl1, PointFlag::CONTROL_POINT_1, true);
    push_point(to, PointFlag::ON_CURVE_POINT, true);
}

void Contour::transform(const Transform2 &transform) {
    for (int i = 0; i < points.size(); i++) {
        auto &point = points[i];
        point = transform * point;

        union_rect(bounds, point, i == 0);
    }
}

SegmentsIter::SegmentsIter(const std::vector<Vec2F> &_points, const std::vector<PointFlag> &_flags, bool _closed)
    : points(_points), flags(_flags), closed(_closed) {}

Segment SegmentsIter::get_next(bool force_closed) {
    Segment segment;

    auto points_count = points.size();

    // The first point in a segment must be an on-curve point instead of a control point.
    if (flags[head] != PointFlag::ON_CURVE_POINT || head >= points.size()) {
        return segment;
    }

    // Set from point.
    segment.baseline.set_from(points[head]);

    if (head + 1 < points_count) {
        // Line.
        if (flags[head + 1] == PointFlag::ON_CURVE_POINT) {
            segment.baseline.set_to(points[head + 1]);
            segment.kind = SegmentKind::Line;

            // Update head.
            head += 1;
        } else if (head + 2 < points_count) {
            // Quadratic Bézier curve.
            if (flags[head + 1] == PointFlag::CONTROL_POINT_0 && flags[head + 2] == PointFlag::ON_CURVE_POINT) {
                segment.ctrl.set_from(points[head + 1]);
                segment.baseline.set_to(points[head + 2]);
                segment.kind = SegmentKind::Quadratic;

                // Update head.
                head += 2;
            } else if (head + 3 < points_count) {
                // Cubic Bézier curve.
                if (flags[head + 1] == PointFlag::CONTROL_POINT_0 && flags[head + 2] == PointFlag::CONTROL_POINT_1 &&
                    flags[head + 3] == PointFlag::ON_CURVE_POINT) {
                    segment.ctrl.set_from(points[head + 1]);
                    segment.ctrl.set_to(points[head + 2]);
                    segment.baseline.set_to(points[head + 3]);
                    segment.kind = SegmentKind::Cubic;

                    // Update head.
                    head += 3;
                }
            }
        }
    } else { // Reach end.
        // Close contour.
        if ((closed || force_closed) && points.size() > 1) {
            segment.baseline.set_from(points.back());
            segment.baseline.set_to(points.front());
            segment.kind = SegmentKind::Line;
            segment.flags = SegmentFlags::LAST_IN_CONTOUR;
        }

        has_next = false;
    }

    return segment;
}

bool SegmentsIter::is_at_start() const {
    return head == 0;
}

bool SegmentsIter::is_at_end() const {
    return !has_next;
}

} // namespace Pathfinder
