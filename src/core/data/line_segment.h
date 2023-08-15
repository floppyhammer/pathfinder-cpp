#ifndef PATHFINDER_LINE_SEGMENT_H
#define PATHFINDER_LINE_SEGMENT_H

#include "../../common/f32x4.h"
#include "../../common/math/transform2.h"
#include "../../common/math/vec2.h"

namespace Pathfinder {

/// Various flags that specify the relation of this segment to other segments in a contour.
enum class SegmentFlags {
    NONE = 0x00,
    /// This segment is the first one in the contour.
    FIRST_IN_CONTOUR = 0x01,
    /// This segment is the closing segment of the contour (i.e. it returns back to the starting point).
    LAST_IN_CONTOUR = 0x02,
};

enum class SegmentKind {
    None,      // Invalid segment.
    Line,      // Line segment.
    Quadratic, // Quadratic Bézier curve.
    Cubic,     // Cubic Bézier curve.
};

struct LineSegmentU16 {
    uint16_t from_x;
    uint16_t from_y;
    uint16_t to_x;
    uint16_t to_y;
};

struct LineSegmentF {
    F32x4 value = F32x4::splat(0);

    LineSegmentF() = default;

    explicit LineSegmentF(const F32x4 &_value);

    LineSegmentF(const Vec2F &from, const Vec2F &to);

    LineSegmentF(float from_x, float from_y, float to_x, float to_y);

    inline Vec2F from() const {
        return value.xy();
    }

    inline Vec2F to() const {
        return value.zw();
    }

    /// Get line segment vector.
    inline Vec2F vector() const {
        return to() - from();
    }

    inline Vec2F sample(float t) const {
        return from() + vector() * t;
    }

    LineSegmentF apply_transform(const Transform2 &transform) const;

    /// Element-wise clamping.
    LineSegmentF clamp(float min, float max) const;

    /// Element-wise rounding.
    LineSegmentF round() const;

    LineSegmentF reversed() const;

    void split(float t, LineSegmentF &segment0, LineSegmentF &segment1) const;

    float min_x() const;

    float max_x() const;

    float min_y() const;

    float max_y() const;

    void set_from(const Vec2F &point);

    void set_to(const Vec2F &point);

    float square_length() const;

    bool intersection_t(const LineSegmentF &other, float &output) const;

    LineSegmentF offset(float distance) const;

    bool is_valid() const;

    inline LineSegmentF operator+(const Vec2F &v) const {
        return {from() + v, to() + v};
    }

    inline LineSegmentF operator*(const Vec2F &v) const {
        return {from() * v, to() * v};
    }

    // For being used as ordered key.
    inline bool operator<(const LineSegmentF &rhs) const {
        bool res = value.get<0>() < rhs.value.get<0>();
        res = res && value.get<1>() < rhs.value.get<1>();
        res = res && value.get<2>() < rhs.value.get<2>();
        res = res && value.get<3>() < rhs.value.get<3>();
        return res;
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_LINE_SEGMENT_H
