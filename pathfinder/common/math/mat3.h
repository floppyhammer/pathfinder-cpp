#ifndef PATHFINDER_MAT3_H
#define PATHFINDER_MAT3_H

#include "vec2.h"

namespace Pathfinder {

/// A 3x3 matrix, in column-major order.
struct Mat3 {
    float v[9] = {0};

    Mat3() = default;

    explicit Mat3(float s) {
        for (int i = 0; i < 3; i++) {
            v[i * 3 + i] = s;
        }
    }

    static Mat3 from_scale(const Vec2F &scale) {
        auto mat = Mat3(1);

        mat.v[0] = scale.x;
        mat.v[4] = scale.y;

        return mat;
    }

    static Mat3 from_translation(const Vec2F &translation) {
        auto mat = Mat3(1);

        mat.v[6] = translation.x;
        mat.v[7] = translation.y;

        return mat;
    }

    static Mat3 from_rotation(float theta) {
        auto mat = Mat3(1);

        mat.v[0] = cos(theta);
        mat.v[3] = -sin(theta);
        mat.v[1] = sin(theta);
        mat.v[4] = cos(theta);

        return mat;
    }

    Mat3 operator*(const Mat3 &other) const {
        auto mat = Mat3();

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                auto index = i + j * 3;
                mat.v[index] = 0;
                for (int k = 0; k < 3; k++) {
                    mat.v[index] += v[i + k * 3] * other.v[k + j * 3];
                }
            }
        }

        return mat;
    }

    Mat3 translate(const Vec2F &translation) const {
        return *this * from_translation(translation);
    }

    Mat3 scale(const Vec2F &scale) const {
        return *this * from_scale(scale);
    }

    Mat3 rotate(float theta) const {
        return *this * from_rotation(theta);
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_MAT3_H
