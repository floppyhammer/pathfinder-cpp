#ifndef PATHFINDER_MAT4X4_H
#define PATHFINDER_MAT4X4_H

#include "vec3.h"

namespace Pathfinder {

/// A 4x4 matrix, in column-major order.
template <typename T>
struct Mat4x4 {
    T v[16] = {0};

    Mat4x4() = default;

    explicit Mat4x4(T s) {
        for (int i = 0; i < 4; i++) {
            v[i * 4 + i] = s;
        }
    }

    static Mat4x4 from_scale(const Vec3F &p_scale) {
        auto mat = Mat4x4(1);

        mat.v[0] = p_scale.x;
        mat.v[5] = p_scale.y;
        mat.v[10] = p_scale.z;

        return mat;
    }

    static Mat4x4 from_translation(const Vec3F &p_translation) {
        auto mat = Mat4x4(1);

        mat.v[12] = p_translation.x;
        mat.v[13] = p_translation.y;
        mat.v[14] = p_translation.z;

        return mat;
    }

    inline Mat4x4 operator*(const Mat4x4 &other) const {
        auto mat = Mat4x4();

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

    inline Mat4x4 translate(const Vec3<T> &p_translation) const {
        return *this * Mat4x4<float>::from_translation(p_translation);
    }

    inline Mat4x4 scale(const Vec3<T> &p_scale) const {
        return *this * Mat4x4<float>::from_scale(p_scale);
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_MAT4x4_H
