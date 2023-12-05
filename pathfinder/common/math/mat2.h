#ifndef PATHFINDER_MAT2_H
#define PATHFINDER_MAT2_H

#include "unit_vector.h"

namespace Pathfinder {

/// A 2x2 matrix, in column-major order.
struct Mat2 {
    float v[4] = {0};

    Mat2() = default;

    Mat2(float m11, float m21, float m12, float m22) {
        v[0] = m11;
        v[1] = m21;
        v[2] = m12;
        v[3] = m22;
    }

    static Mat2 identity() {
        return Mat2::from_scale(Vec2F(1));
    }

    static Mat2 from_scale(Vec2F scale) {
        return {scale.x, 0.0, 0.0, scale.y};
    }

    static Mat2 from_rotation(float theta) {
        return Mat2::from_rotation_vector(UnitVector::from_angle(theta));
    }

    static Mat2 from_rotation_vector(const UnitVector &vector) {
        return {1.0f * vector.x, 1.0f * vector.y, -1.0f * vector.y, 1.0f * vector.x};
    }

    inline float det() const {
        return v[0] * v[3] - v[1] * v[2];
    }

    inline Mat2 adjugate() const {
        return {v[3] * 1.0f, v[1] * -1.0f, v[2] * -1.0f, v[0] * 1.0f};
    }

    inline Mat2 inverse() const {
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

    inline Mat2 operator*(float s) const {
        return {v[0] * s, v[1] * s, v[2] * s, v[3] * s};
    }

    inline Mat2 operator*(const Mat2 &other) const {
        return {v[0] * other.v[0] + v[2] * other.v[1],
                v[1] * other.v[0] + v[3] * other.v[1],
                v[0] * other.v[2] + v[2] * other.v[3],
                v[1] * other.v[2] + v[3] * other.v[3]};
    }

    inline bool operator==(const Mat2 &b) const {
        return v[0] == b.v[0] && v[1] == b.v[1] && v[2] == b.v[2] && v[3] == b.v[3];
    }

    // For being used as ordered key.
    inline bool operator<(const Mat2 &b) const {
        return v[0] < b.v[0] && v[1] < b.v[1] && v[2] < b.v[2] && v[3] < b.v[3];
    }

    inline Vec2F operator*(const Vec2F &other) const {
        return {v[0] * other.x + v[2] * other.y, v[1] * other.x + v[3] * other.y};
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_MAT2_H
