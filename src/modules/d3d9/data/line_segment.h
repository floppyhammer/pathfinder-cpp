//
// Created by floppyhammer on 7/9/2021.
//

#ifndef PATHFINDER_D3D9_LINE_SEGMENT_H
#define PATHFINDER_D3D9_LINE_SEGMENT_H

#include "../../../common/math/vec2.h"
#include "../../../common/f32x4.h"

namespace Pathfinder {
    /// Various flags that specify the relation of this segment to other segments in a path.
    enum class SegmentFlags {
        NONE = 0x00,
        /// This segment is the first one in the contour.
        FIRST_IN_PATH = 0x01,
        /// This segment is the closing segment of the path (i.e. it returns back to the starting point).
        LAST_IN_PATH = 0x02,
    };

    enum class SegmentKind {
        None, // Invalid segment.
        Line, // Line segment.
        Quadratic, // Quadratic Bézier curve.
        Cubic, // Cubic Bézier curve.
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

        explicit LineSegmentF(const F32x4 &p_value);

        LineSegmentF(const Vec2<float> &p_from, const Vec2<float> &p_to);

        LineSegmentF(float p_from_x, float p_from_y, float p_to_x, float p_to_y);

        inline Vec2<float> from() const {
            return value.xy();
        }

        inline Vec2<float> to() const {
            return value.zw();
        }

        /// Get line segment vector.
        inline Vec2<float> vector() const {
            return to() - from();
        }

        inline Vec2<float> sample(float t) const {
            return from() + vector() * t;
        }

        LineSegmentF clamp(float min, float max) const;

        LineSegmentF round() const;

        LineSegmentF reversed() const;

        void split(float t, LineSegmentF &segment0, LineSegmentF &segment1) const;

        float min_x() const;

        float max_x() const;

        float min_y() const;

        float max_y() const;

        void set_from(const Vec2<float> &point);

        void set_to(const Vec2<float> &point);

        float square_length() const;

        bool intersection_t(const LineSegmentF &other, float &output) const;

        LineSegmentF offset(float distance) const;

        inline LineSegmentF operator+(const Vec2<float> &v) const {
            return {from() + v, to() + v};
        }

        inline LineSegmentF operator*(const Vec2<float> &v) const {
            return {from() * v, to() * v};
        }
    };
}

#endif //PATHFINDER_D3D9_LINE_SEGMENT_H
