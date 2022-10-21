#ifndef PATHFINDER_MAT2X2_H
#define PATHFINDER_MAT2X2_H

#include "unit_vector.h"

namespace Pathfinder {

/// A 2x2 matrix, in column-major order.
/// | x z |
/// | y w |
template <typename T>
struct Mat2x2 {
    T v[4] = {0}; // x, y, z, w.

    Mat2x2() = default;

    Mat2x2(T m11, T m21, T m12, T m22) {
        v[0] = m11;
        v[1] = m21;
        v[2] = m12;
        v[3] = m22;
    }

    static Mat2x2 identity() {
        return Mat2x2<float>::from_scale(Vec2F(1));
    }

    static Mat2x2 from_scale(Vec2F p_scale) {
        return {p_scale.x, 0.0, 0.0, p_scale.y};
    }

    static Mat2x2 from_rotation(float theta) {
        return Mat2x2::from_rotation_vector(UnitVector::from_angle(theta));
    }

    static Mat2x2 from_rotation_vector(const UnitVector &p_vector) {
        return {1.0f * p_vector.x, 1.0f * p_vector.y, -1.0f * p_vector.y, 1.0f * p_vector.x};
    }

    inline float det() const {
        return v[0] * v[3] - v[1] * v[2];
    }

    inline Mat2x2 adjugate() const {
        return {v[3] * 1.0f, v[1] * -1.0f, v[2] * -1.0f, v[0] * 1.0f};
    }

    inline Mat2x2 inverse() const {
        return adjugate() * (1.0f / det());
    }

    inline float m11() const {
        return v[0];
    }

    inline float m21() const {
        return v[1];
    }

    inline float m12() const {
        return v[2];
    }

    inline float m22() const {
        return v[3];
    }

    inline Mat2x2 operator*(float s) const {
        return {v[0] * s, v[1] * s, v[2] * s, v[3] * s};
    }

    inline Mat2x2 operator*(const Mat2x2 &other) const {
        return {v[0] * other.v[0] + v[2] * other.v[1],
                v[1] * other.v[0] + v[3] * other.v[1],
                v[0] * other.v[2] + v[2] * other.v[3],
                v[1] * other.v[2] + v[3] * other.v[3]};
    }

    inline bool operator==(const Mat2x2 &b) const {
        return v[0] == b.v[0] && v[1] == b.v[1] && v[2] == b.v[2] && v[3] == b.v[3];
    }

    inline Vec2F operator*(const Vec2F &other) const {
        return {v[0] * other.x + v[2] * other.y, v[1] * other.x + v[3] * other.y};
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_MAT2X2_H
