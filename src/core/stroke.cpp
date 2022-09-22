#include "stroke.h"

#include <algorithm>
#include <utility>

namespace Pathfinder {
/// Stroking tolerance.
const float STROKE_TOL = 0.1f;

const uint32_t SAMPLE_COUNT = 16;

const float EPSILON = 0.001;

ContourStrokeToFill::ContourStrokeToFill(Contour p_input, float p_radius, LineJoin p_join, float p_join_miter_limit)
    : input(std::move(p_input)), radius(p_radius), join(p_join), join_miter_limit(p_join_miter_limit) {
}

void ContourStrokeToFill::offset_forward() {
    auto segments_iter = SegmentsIter(input.points, input.flags, input.closed);

    // Traverse curve/line segments.
    while (!segments_iter.is_at_end()) {
        // Get next segment in the contour.
        auto segment = segments_iter.get_next();

        // Invalid segment.
        if (segment.kind == SegmentKind::None) break;

        // FIXME(pcwalton): We negate the radius here so that round end caps can be drawn clockwise.
        // Of course, we should just implement anticlockwise arcs to begin with...
        LineJoin line_join = segments_iter.is_at_start() ? LineJoin::Bevel : join;

        segment.offset(-radius, line_join, join_miter_limit, output);
    }
}

void ContourStrokeToFill::offset_backward() {
    auto segments = input.get_segments();

    std::reverse(segments.begin(), segments.end());

    for (int segment_index = 0; segment_index < segments.size(); segment_index++) {
        auto segment = segments[segment_index].reversed();

        // FIXME(pcwalton): We negate the radius here so that round end caps can be drawn clockwise.
        // Of course, we should just implement anticlockwise arcs to begin with...
        LineJoin line_join = segment_index == 0 ? LineJoin::Bevel : join;

        segment.offset(-radius, line_join, join_miter_limit, output);
    }
}

OutlineStrokeToFill::OutlineStrokeToFill(const Outline &p_input, StrokeStyle p_style) : input(p_input), style(p_style) {
}

void OutlineStrokeToFill::offset() {
    // Resulting contours.
    std::vector<Contour> new_contours;

    // Convert each contour.
    for (auto &contour : input.contours) {
        auto closed = contour.closed;

        // Note that we need to pass radius instead of width.
        auto stroker = ContourStrokeToFill(contour, style.line_width * 0.5f, style.line_join, style.miter_limit);

        // Scale the contour up, forming an outer contour.
        stroker.offset_forward();

        // If closed (easy case), we can just use even-odd fill rule on the outer and inner contours to get the stroke
        // fill.
        if (closed) {
            push_stroked_contour(new_contours, stroker, true);
            stroker = ContourStrokeToFill(contour, style.line_width * 0.5f, style.line_join, style.miter_limit);
        } else { // If not closed (hard case), we need to connect the outer and inner contours into a single contour.
            add_cap(stroker.output);
        }

        // Scale the contour down, forming an inner contour.
        stroker.offset_backward();

        // If not closed, we need to connect the outer and inner contours.
        if (!closed) {
            add_cap(stroker.output);
        }

        push_stroked_contour(new_contours, stroker, closed);
    }

    Rect<float> new_bounds;

    for (auto &p : new_contours) {
        p.update_bounds(new_bounds);
    }

    output.contours = new_contours;
    output.bounds = new_bounds;
}

Outline OutlineStrokeToFill::into_outline() const {
    return output;
}

void OutlineStrokeToFill::push_stroked_contour(std::vector<Contour> &new_contours,
                                               ContourStrokeToFill stroker,
                                               bool closed) const {
    // Add join if necessary.
    if (closed && stroker.output.might_need_join(style.line_join)) {
        auto p1 = stroker.output.points[1];
        auto p0 = stroker.output.points[0];

        auto final_segment = LineSegmentF(p1, p0);
        stroker.output.add_join(
            style.line_width * 0.5f, style.line_join, stroker.input.points[0], final_segment, style.miter_limit);
    }

    stroker.output.closed = true;
    new_contours.push_back(stroker.output);
}

void OutlineStrokeToFill::add_cap(Contour &contour) const {
    // If cap is butt, the two points in the outer and inner contour can
    // connect to each other automatically because of the shared
    // ON_CURVE_POINT flag.
    if (style.line_cap == LineCap::Butt || contour.points.size() < 2) {
        return;
    }

    auto width = style.line_width;
    auto p1 = contour.position_of_last(1);

    // Determine the ending gradient.
    Vec2<float> p0{};
    auto p0_index = contour.points.size() - 2;

    while (true) {
        p0 = contour.points[p0_index];
        if ((p1 - p0).square_length() > EPSILON) {
            break;
        }
        if (p0_index == 0) {
            return;
        }
        p0_index -= 1;
    }

    auto gradient = (p1 - p0).normalize();

    switch (style.line_cap) {
        case LineCap::Butt:
            break;
        case LineCap::Square: {
            auto offset = gradient * (width * 0.5f);

            auto p2 = p1 + offset;
            auto p3 = p2 + gradient.yx() * Vec2<float>(-width, width);
            auto p4 = p3 - offset;

            contour.push_endpoint(p2);
            contour.push_endpoint(p3);
            contour.push_endpoint(p4);
        } break;
        case LineCap::Round: {
            auto scale = width * 0.5f;
            auto offset = gradient.yx() * Vec2<float>(-1.0, 1.0);
            auto translation = p1 + offset * (width * 0.5f);
            auto transform = Transform2::from_scale(Vec2<float>(scale, scale)).translate(translation);
            auto chord = LineSegmentF(-offset, offset);

            contour.push_arc_from_unit_chord(transform, chord, ArcDirection::CW);
        } break;
    }
}

bool Contour::is_empty() const {
    return points.empty();
}

bool Contour::might_need_join(LineJoin join) const {
    // A single line has no join.
    if (points.size() < 2) {
        return false;
    } else {
        if (join == LineJoin::Miter || join == LineJoin::Round)
            return true;
        else
            return false;
    }
}

void Contour::add_join(
    float distance, LineJoin join, Vec2<float> join_point, LineSegmentF next_tangent, float miter_limit) {
    auto p0 = position_of_last(2);
    auto p1 = position_of_last(1);

    auto prev_tangent = LineSegmentF(p0, p1);

    if (prev_tangent.square_length() < EPSILON || next_tangent.square_length() < EPSILON) {
        return;
    }

    switch (join) {
        case LineJoin::Bevel:
            break;
        case LineJoin::Miter: {
            float prev_tangent_t;
            auto valid = prev_tangent.intersection_t(next_tangent, prev_tangent_t);
            if (!valid) return;

            if (prev_tangent_t < -EPSILON) {
                return;
            }

            auto miter_endpoint = prev_tangent.sample(prev_tangent_t);
            auto threshold = miter_limit * distance;
            if ((miter_endpoint - join_point).square_length() > threshold * threshold) {
                return;
            }
            push_endpoint(miter_endpoint);
        } break;
        case LineJoin::Round: {
            auto scale = std::abs(distance);
            auto transform = Transform2::from_scale(Vec2<float>(scale)).translate(join_point);
            auto chord_from = (prev_tangent.to() - join_point).normalize();
            auto chord_to = (next_tangent.to() - join_point).normalize();
            auto chord = LineSegmentF(chord_from, chord_to);
            push_arc_from_unit_chord(transform, chord, ArcDirection::CW);
        } break;
    }
}

void Contour::push_arc_from_unit_chord(Transform2 &transform, LineSegmentF chord, const ArcDirection direction) {
    auto direction_transform = Transform2();
    if (direction == ArcDirection::CCW) {
        chord = chord * Vec2<float>(1.0f, -1.0f);
        direction_transform = Transform2::from_scale(Vec2<float>(1.0, -1.0));
    }

    auto vector = UnitVector(chord.from());
    auto end_vector = UnitVector(chord.to());

    for (int segment_index = 0; segment_index < 4; segment_index++) {
        auto sweep_vector = end_vector.rev_rotate_by(vector);
        const auto last = sweep_vector.x >= -EPSILON && sweep_vector.y >= -EPSILON;

        Segment segment;
        if (!last) {
            sweep_vector = UnitVector(Vec2<float>(0.0f, 1.0f));
            segment = Segment::quarter_circle_arc();
        } else {
            segment = Segment::arc_from_cos(sweep_vector.x);
        }

        auto half_sweep_vector = sweep_vector.halve_angle();
        auto rotation = Transform2::from_rotation_vector(half_sweep_vector.rotate_by(vector));
        segment = segment.transform(transform * direction_transform * rotation);

        auto push_segment_flags = PushSegmentFlags(PushSegmentFlags::UPDATE_BOUNDS);
        if (segment_index == 0) {
            push_segment_flags.value |= PushSegmentFlags::INCLUDE_FROM_POINT;
        }

        push_segment(segment, push_segment_flags);

        if (last) break;

        vector = vector.rotate_by(sweep_vector);
    }
}

Segment Segment::offset_once(float distance) const {
    // Handle line.
    if (is_line()) {
        return Segment::line(baseline.offset(distance));
    }

    // Handle quadratic curve.
    if (is_quadratic()) {
        auto segment_0 = LineSegmentF(baseline.from(), ctrl.from());
        auto segment_1 = LineSegmentF(ctrl.from(), baseline.to());
        segment_0 = segment_0.offset(distance);
        segment_1 = segment_1.offset(distance);

        float t;
        Vec2<float> local_ctrl =
            segment_0.intersection_t(segment_1, t) ? segment_0.sample(t) : lerp(segment_0.to(), segment_1.from(), 0.5f);

        auto local_baseline = LineSegmentF(segment_0.from(), segment_1.to());
        return Segment::quadratic(local_baseline, local_ctrl);
    }

    // Handle cubic curve.
    if (baseline.from() == ctrl.from()) {
        auto segment_0 = LineSegmentF(baseline.from(), ctrl.to());
        auto segment_1 = LineSegmentF(ctrl.to(), baseline.to());
        segment_0 = segment_0.offset(distance);
        segment_1 = segment_1.offset(distance);

        float t;
        Vec2<float> local_ctrl =
            segment_0.intersection_t(segment_1, t) ? segment_0.sample(t) : lerp(segment_0.to(), segment_1.from(), 0.5f);

        auto local_baseline = LineSegmentF(segment_0.from(), segment_1.to());
        auto local_ctrl2 = LineSegmentF(segment_0.from(), local_ctrl);
        return Segment::cubic(local_baseline, local_ctrl2);
    }

    if (ctrl.to() == baseline.to()) {
        auto segment_0 = LineSegmentF(baseline.from(), ctrl.from());
        auto segment_1 = LineSegmentF(ctrl.from(), baseline.to());
        segment_0 = segment_0.offset(distance);
        segment_1 = segment_1.offset(distance);

        float t;
        Vec2<float> local_ctrl =
            segment_0.intersection_t(segment_1, t) ? segment_0.sample(t) : lerp(segment_0.to(), segment_1.from(), 0.5f);

        auto local_baseline = LineSegmentF(segment_0.from(), segment_1.to());
        auto local_ctrl2 = LineSegmentF(local_ctrl, segment_1.to());
        return Segment::cubic(local_baseline, local_ctrl2);
    }

    auto segment_0 = LineSegmentF(baseline.from(), ctrl.from());
    auto segment_1 = LineSegmentF(ctrl.from(), ctrl.to());
    auto segment_2 = LineSegmentF(ctrl.to(), baseline.to());
    segment_0 = segment_0.offset(distance);
    segment_1 = segment_1.offset(distance);
    segment_2 = segment_2.offset(distance);

    Vec2<float> ctrl_0, ctrl_1;
    float t0, t1;
    if (segment_0.intersection_t(segment_1, t0) && segment_1.intersection_t(segment_2, t1)) {
        ctrl_0 = segment_0.sample(t0);
        ctrl_1 = segment_1.sample(t1);
    } else {
        ctrl_0 = lerp(segment_0.to(), segment_1.from(), 0.5f);
        ctrl_1 = lerp(segment_1.to(), segment_2.from(), 0.5f);
    }

    auto local_baseline = LineSegmentF(segment_0.from(), segment_2.to());
    auto local_ctrl = LineSegmentF(ctrl_0, ctrl_1);
    return Segment::cubic(local_baseline, local_ctrl);
}

Segment Segment::reversed() const {
    LineSegmentF new_ctrl;
    if (is_quadratic()) {
        new_ctrl = ctrl;
    } else {
        new_ctrl = ctrl.reversed();
    }

    return {baseline.reversed(), new_ctrl, kind, flags};
}

bool Segment::error_is_within_tolerance(const Segment &other, float distance) const {
    auto min = std::abs(distance) - STROKE_TOL;
    auto max = std::abs(distance) + STROKE_TOL;

    min = min <= 0 ? 0.0f : min * min;
    max = max <= 0 ? 0.0f : max * max;

    for (int t_num = 0; t_num < SAMPLE_COUNT + 1; t_num++) {
        auto t = (float)t_num / (float)SAMPLE_COUNT;
        // FIXME(pcwalton): Use signed distance!
        auto this_p = sample(t);
        auto other_p = other.sample(t);

        auto vector = this_p - other_p;
        auto square_distance = vector.square_length();

        if (square_distance < min || square_distance > max) {
            return false;
        }
    }

    return true;
}

void Segment::add_to_contour(
    float distance, LineJoin join, Vec2<float> join_point, float join_miter_limit, Contour &contour) const {
    // Add join if necessary.
    if (contour.might_need_join(join)) {
        auto p3 = baseline.from();

        // NB: If you change the representation of quadratic curves,
        // you will need to change this.
        Vec2<float> p4 = is_line() ? baseline.to() : ctrl.from();

        contour.add_join(distance, join, join_point, LineSegmentF(p4, p3), join_miter_limit);
    }

    // Push segment.
    PushSegmentFlags push_flags;
    push_flags.value = PushSegmentFlags::UPDATE_BOUNDS | PushSegmentFlags::INCLUDE_FROM_POINT;
    contour.push_segment(*this, push_flags);
}

void Segment::offset(float distance, LineJoin join, float join_miter_limit, Contour &contour) const {
    auto join_point = baseline.from();
    if (baseline.square_length() < STROKE_TOL * STROKE_TOL) {
        add_to_contour(distance, join, join_point, join_miter_limit, contour);
        return;
    }

    auto candidate = offset_once(distance);
    if (error_is_within_tolerance(candidate, distance)) {
        candidate.add_to_contour(distance, join, join_point, join_miter_limit, contour);
        return;
    }

    Segment before, after;
    split(0.5f, before, after);

    before.offset(distance, join, join_miter_limit, contour);
    after.offset(distance, join, join_miter_limit, contour);
}
} // namespace Pathfinder
