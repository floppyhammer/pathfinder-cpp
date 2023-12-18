#ifndef PATHFINDER_MAT4_H
#define PATHFINDER_MAT4_H

#include "vec3.h"

namespace Pathfinder {

/// A 4x4 matrix, in column-major order.
struct Mat4 {
    float v[16] = {0};

    Mat4() = default;

    explicit Mat4(float s) {
        for (int i = 0; i < 4; i++) {
            v[i * 4 + i] = s;
        }
    }

    static Mat4 from_scale(const Vec3F &scale) {
        auto mat = Mat4(1);

        mat.v[0] = scale.x;
        mat.v[5] = scale.y;
        mat.v[10] = scale.z;

        return mat;
    }

    static Mat4 from_translation(const Vec3F &translation) {
        auto mat = Mat4(1);

        mat.v[12] = translation.x;
        mat.v[13] = translation.y;
        mat.v[14] = translation.z;

        return mat;
    }

    Mat4 operator*(const Mat4 &other) const {
        auto mat = Mat4();

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                auto index = i + j * 4;
                mat.v[index] = 0;
                for (int k = 0; k < 4; k++) {
                    mat.v[index] += v[i + k * 4] * other.v[k + j * 4];
                }
            }
        }

        return mat;
    }

    Mat4 translate(const Vec3F &translation) const {
        return *this * from_translation(translation);
    }

    Mat4 scale(const Vec3F &scale) const {
        return *this * from_scale(scale);
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_MAT4_H
