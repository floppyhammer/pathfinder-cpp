#include "line_segment.h"

#include <cassert>
#include <cmath>

#include "../../common/math/basic.h"
#include "../../common/math/mat2x2.h"

namespace Pathfinder {

LineSegmentF::LineSegmentF(const F32x4 &_value) : value(_value) {}

LineSegmentF::LineSegmentF(const Vec2F &from, const Vec2F &to) : value(F32x4(from.x, from.y, to.x, to.y)) {}

LineSegmentF::LineSegmentF(float from_x, float from_y, float to_x, float to_y)
    : value(F32x4(from_x, from_y, to_x, to_y)) {}

LineSegmentF LineSegmentF::clamp(float min, float max) const {
    F32x4 result = value.clamp(F32x4::splat(min), F32x4::splat(max));

    return LineSegmentF{result};
}

LineSegmentF LineSegmentF::round() const {
    auto result = F32x4(std::round(value.get<0>()),
                        std::round(value.get<1>()),
                        std::round(value.get<2>()),
                        std::round(value.get<3>()));

    return LineSegmentF(result);
}

LineSegmentF LineSegmentF::reversed() const {
    return LineSegmentF(value.zwxy());
}

void LineSegmentF::split(float t, LineSegmentF &segment0, LineSegmentF &segment1) const {
    assert(t >= 0.0 && t <= 1.0);

    auto from_from = value.xyxy();
    auto to_to = value.zwzw();

    auto d_d = to_to - from_from;
    auto mid_mid = from_from + d_d * F32x4::splat(t);

    segment0.value = from_from.concat_xy_xy(mid_mid);
    segment1.value = mid_mid.concat_xy_xy(to_to);
}

float LineSegmentF::min_x() const {
    return std::min(from().x, to().x);
}

float LineSegmentF::max_x() const {
    return std::max(from().x, to().x);
}

float LineSegmentF::min_y() const {
    return std::min(from().y, to().y);
}

float LineSegmentF::max_y() const {
    return std::max(from().y, to().y);
}

void LineSegmentF::set_from(const Vec2F &point) {
    value = F32x4(point.x, point.y, value.zw().x, value.zw().y);
}

void LineSegmentF::set_to(const Vec2F &point) {
    value = F32x4(value.xy().x, value.xy().y, point.x, point.y);
}

float LineSegmentF::square_length() const {
    return (to() - from()).square_length();
}

bool LineSegmentF::intersection_t(const LineSegmentF &other, float &output) const {
    const float EPSILON = 0.0001;

    auto p0p1 = vector();
    auto matrix = Mat2x2<float>(other.vector().x, other.vector().y, -p0p1.x, -p0p1.y);
    if (std::abs(matrix.det()) < EPSILON) {
        return false;
    }

    output = (matrix.inverse() * (from() - other.from())).y;
    return true;
}

LineSegmentF LineSegmentF::offset(float distance) const {
    if (vector().is_zero()) {
        return *this;
    } else {
        return *this + vector().yx().normalize() * Vec2F(-distance, distance);
    }
}

bool LineSegmentF::check_validity() const {
    if (std::isnan(value.get<0>()) || std::isnan(value.get<1>()) || std::isnan(value.get<2>()) ||
        std::isnan(value.get<3>())) {
        Logger::error("NaN encountered!", "LineSegmentF");
        return false;
    }
    return true;
}

} // namespace Pathfinder
