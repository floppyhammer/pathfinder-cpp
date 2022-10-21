#ifndef PATHFINDER_TRANSFORM2_H
#define PATHFINDER_TRANSFORM2_H

#include "mat2x2.h"
#include "rect.h"
#include "unit_vector.h"
#include "vec2.h"

namespace Pathfinder {

struct Transform2 {
private:
    Mat2x2<float> matrix;
    Vec2F vector;

public:
    Transform2();

    explicit Transform2(float xform[6]);

    Transform2(Mat2x2<float> p_matrix, Vec2F p_vector);

    static Transform2 from_scale(const Vec2F &scale) {
        return {Mat2x2<float>::from_scale(scale), Vec2F(0)};
    }

    static Transform2 from_rotation(float theta) {
        return {Mat2x2<float>::from_rotation(theta), Vec2F(0)};
    }

    static Transform2 from_translation(const Vec2F &translation) {
        return {Mat2x2<float>::identity(), translation};
    }

    static Transform2 from_rotation_vector(const UnitVector &p_vector) {
        return {Mat2x2<float>::from_rotation_vector(p_vector), Vec2F(0)};
    }

    bool is_identity() const;

    Transform2 inverse() const;

    /**
     * Left-multiply a translation matrix.
     * @param p_vector Translation
     * @return New transform
     */
    Transform2 translate(Vec2F p_vector) const;

    /**
     * Left-multiply a rotation matrix.
     * @param theta Rotation in radian
     * @return New transform
     */
    Transform2 rotate(float theta) const;

    Vec2F get_position() const;

    inline float m11() const {
        return matrix.m11();
    }

    inline float m21() const {
        return matrix.m21();
    }

    inline float m12() const {
        return matrix.m12();
    }

    inline float m22() const {
        return matrix.m22();
    }

    inline float m13() const {
        return vector.x;
    }

    inline float m23() const {
        return vector.y;
    }

    inline Vec2F operator*(const Vec2F &p_vector) const {
        return {matrix * p_vector + vector};
    }

    inline RectF operator*(const RectF &p_rect) const {
        auto upper_left = *this * p_rect.origin();
        auto upper_right = *this * p_rect.upper_right();
        auto lower_left = *this * p_rect.lower_left();
        auto lower_right = *this * p_rect.lower_right();
        auto min_point = upper_left.min(upper_right).min(lower_left).min(lower_right);
        auto max_point = upper_left.max(upper_right).max(lower_left).max(lower_right);

        return {min_point, max_point};
    }

    inline Transform2 operator*(const Transform2 &other) const {
        return {matrix * other.matrix, *this * other.vector};
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_TRANSFORM2_H
