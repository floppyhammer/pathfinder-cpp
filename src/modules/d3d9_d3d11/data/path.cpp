//
// Created by chy on 7/9/2021.
//

#include "path.h"

#include <cassert>

namespace Pathfinder {
    Vec2<float> Path::position_of_last(int index) {
        return points[points.size() - index];
    }

    void Path::push_point(Vec2<float> point, PointFlags p_flag, bool update_bounds) {
        if (update_bounds) {
            auto first = points.empty();
            union_rect(bounds, point, first);
        }

        points.push_back(point);
        flags.push_back(p_flag);
    }

    void Path::push_endpoint(Vec2<float> to) {
        push_point(to, PointFlags{}, true);
    }

    void Path::push_segment(const Segment &segment, PushSegmentFlags p_flags) {
        // Not a valid segment.
        if (segment.kind == SegmentKind::None) {
            return;
        }

        auto update_bounds = (p_flags.value & PushSegmentFlags::UPDATE_BOUNDS) > 0x00;

        // Push the first on-curve point.
        push_point(segment.baseline.from(), PointFlags(), update_bounds);

        // Not a line.
        if (!segment.is_line()) {
            // Push the first control point.
            push_point(segment.ctrl.from(), PointFlags(PointFlags::CONTROL_POINT_0), update_bounds);

            // Not a quadratic curve.
            if (!segment.is_quadratic()) {
                // Push the second control point.
                push_point(segment.ctrl.to(), PointFlags(PointFlags::CONTROL_POINT_1), update_bounds);
            }
        }

        // Push the second on-curve point.
        push_point(segment.baseline.to(), PointFlags(), update_bounds);
    }

    void Path::update_bounds(Rect<float> &p_bounds) const {
        // The bounds to union is not valid.
        if (!p_bounds.is_valid()) {
            p_bounds = bounds;
        }
        p_bounds = p_bounds.union_rect(bounds);
    }

    std::vector<Segment> Path::get_segments(bool force_closed) const {
        std::vector<Segment> segments;

        auto segments_iter = SegmentsIter(points, flags, closed);

        // Traverse curve/line segments.
        while (!segments_iter.is_at_end()) {
            auto segment = segments_iter.get_next(force_closed);
            if (segment.kind == SegmentKind::None)
                break;
            segments.push_back(segment);
        }

        return segments;
    }

    SegmentsIter::SegmentsIter(const std::vector<Vec2<float>> &p_points,
                               const std::vector<PointFlags> &p_flags,
                               bool p_closed) : points(p_points), flags(p_flags), closed(p_closed) {}

    Segment SegmentsIter::get_next(bool force_closed) {
        Segment segment;

        auto points_count = points.size();

        // The first point in a segment must be an on-curve point instead of a control point.
        if (flags[head].value != 0 || head >= points.size()) {
            return segment;
        }

        // Set from point.
        segment.baseline.set_from(points[head]);

        if (head + 1 < points_count) {
            // Line.
            if (flags[head + 1].value == 0) {
                segment.baseline.set_to(points[head + 1]);
                segment.kind = SegmentKind::Line;

                // Update head.
                head += 1;
            } else if (head + 2 < points_count) {
                // Quadratic Bézier curve.
                if (flags[head + 1].value == PointFlags::CONTROL_POINT_0
                    && flags[head + 2].value == 0) {
                    segment.ctrl.set_from(points[head + 1]);
                    segment.baseline.set_to(points[head + 2]);
                    segment.kind = SegmentKind::Quadratic;

                    // Update head.
                    head += 2;
                } else if (head + 3 < points_count) {
                    // Cubic Bézier curve.
                    if (flags[head + 1].value == PointFlags::CONTROL_POINT_0
                        && flags[head + 2].value == PointFlags::CONTROL_POINT_1
                        && flags[head + 3].value == 0) {
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
            // Close path.
            if ((closed || force_closed) && points.size() > 1) {
                segment.baseline.set_from(points.back());
                segment.baseline.set_to(points.front());
                segment.kind = SegmentKind::Line;
                segment.flags = SegmentFlags::LAST_IN_PATH;
            }

            has_next = false;
        }

        return segment;
    }

    bool SegmentsIter::is_at_start() {
        return head == 0;
    }

    bool SegmentsIter::is_at_end() {
        return !has_next;
    }
}
