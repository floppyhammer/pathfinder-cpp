#include "segment.h"

namespace Pathfinder {

Segment::Segment(const LineSegmentF& _baseline, const LineSegmentF& _ctrl) : baseline(_baseline), ctrl(_ctrl) {}

Segment::Segment(const LineSegmentF& _baseline, const LineSegmentF& _ctrl, SegmentKind _kind, SegmentFlags _flags)
    : baseline(_baseline), ctrl(_ctrl), kind(_kind), flags(_flags) {}

bool Segment::is_flat(float tol) const {
    F32x4 uv = F32x4::splat(3.0) * ctrl.value - baseline.value - baseline.value - baseline.value.zwxy();

    uv = uv * uv;

    uv = uv.max(uv.zwxy());

    return uv.get<0>() + uv.get<1>() <= tol;
}

LineSegmentF Segment::as_line_segment() const {
    return baseline;
}

void Segment::split_cubic(float t, Segment& segment0, Segment& segment1) const {
    if (t <= 0.0) {
        auto from = baseline.from();
        segment0.baseline.set_from(from);
        segment0.baseline.set_to(from);
        segment0.ctrl.set_from(from);
        segment0.ctrl.set_to(from);

        segment1.baseline = baseline;
        segment1.ctrl = ctrl;
    } else if (t >= 1.0) {
        segment0.baseline = baseline;
        segment0.ctrl = ctrl;

        auto to = baseline.to();
        segment1.baseline.set_from(to);
        segment1.baseline.set_to(to);
        segment1.ctrl.set_from(to);
        segment1.ctrl.set_to(to);
    } else {
        auto tttt = F32x4::splat(t);

        auto p0p3 = baseline.value;
        auto p1p2 = ctrl.value;

        auto p0p1 = p0p3.concat_xy_xy(p1p2);

        // p01 = lerp(p0, p1, t), p12 = lerp(p1, p2, t), p23 = lerp(p2, p3, t)
        auto p01p12 = p0p1 + tttt * (p1p2 - p0p1);
        auto pxxp23 = p1p2 + tttt * (p0p3 - p1p2);
        auto p12p23 = p01p12.concat_zw_zw(pxxp23);

        // p012 = lerp(p01, p12, t), p123 = lerp(p12, p23, t)
        auto p012p123 = p01p12 + tttt * (p12p23 - p01p12);
        auto p123 = p012p123.zwzw();

        // p0123 = lerp(p012, p123, t)
        auto p0123 = p012p123 + tttt * (p123 - p012p123);

        // segment0.baseline.set_from(p0);
        // segment0.baseline.set_to(p0123);
        segment0.baseline.value = p0p3.concat_xy_xy(p0123);

        // segment0.ctrl.set_from(p01);
        // segment0.ctrl.set_to(p012);
        segment0.ctrl.value = p01p12.concat_xy_xy(p012p123);

        // segment1.baseline.set_from(p0123);
        // segment1.baseline.set_to(p3);
        segment1.baseline.value = p0123.concat_xy_zw(p0p3);

        // segment1.ctrl.set_from(p123);
        // segment1.ctrl.set_to(p23);
        segment1.ctrl.value = p012p123.concat_zw_zw(p12p23);
    }

    segment0.kind = SegmentKind::Cubic;
    segment1.kind = SegmentKind::Cubic;
}

void Segment::split(float t, Segment& segment0, Segment& segment1) const {
    // Split line.
    if (is_line()) {
        as_line_segment().split(t, segment0.baseline, segment1.baseline);
        segment0.kind = SegmentKind::Line;
        segment1.kind = SegmentKind::Line;
    } else { // Split cubic. Convert quadratic to cubic first.
        to_cubic().split_cubic(t, segment0, segment1);
    }
}

Segment Segment::to_cubic() const {
    if (is_cubic()) {
        return *this;
    }

    auto new_segment = *this;
    auto p1_2 = ctrl.from() + ctrl.from();
    new_segment.ctrl = LineSegmentF((baseline.from() + p1_2) / 3.0f, (p1_2 + baseline.to()) / 3.0f);
    new_segment.kind = SegmentKind::Cubic;
    return new_segment;
}

Segment Segment::transform(Transform2 transform) const {
    return {LineSegmentF(transform * baseline.from(), transform * baseline.to()),
            LineSegmentF(transform * ctrl.from(), transform * ctrl.to()),
            kind,
            flags};
}

Vec2F Segment::sample(float t) const {
    Segment a, b;
    split(t, a, b);

    return a.baseline.to();
}

/// FIXME: Make this more accurate for curves.
float Segment::arc_length() const {
    // Convert quadratic to cubic first.
    auto cubic = kind == SegmentKind::Line ? Segment() : to_cubic();

    switch (kind) {
        case SegmentKind::Line:
            return baseline.vector().length();
        case SegmentKind::Quadratic:
        case SegmentKind::Cubic: {
            // A quick way to get the approximate length of a cubic Bezier curve.
            // See https://community.khronos.org/t/3d-cubic-bezier-segment-length/62363.
            auto chord = cubic.baseline.vector().length();
            auto cont_net = (cubic.baseline.from() - cubic.ctrl.from()).length() + cubic.ctrl.vector().length() +
                            (cubic.baseline.to() - cubic.ctrl.to()).length();
            return (cont_net + chord) * 0.5f;
        }
        default:
            return 0;
    }
}

} // namespace Pathfinder
