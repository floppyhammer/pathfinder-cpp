//
// Created by chy on 2021/11/29.
//

#ifndef PATHFINDER_VEC3_H
#define PATHFINDER_VEC3_H

namespace Pathfinder {
    template<typename T>
    struct Vec3 {
        T x = 0;
        T y = 0;
        T z = 0;

        Vec3() = default;

        explicit Vec3(T p_s) : x(p_s), y(p_s), z(p_s) {};

        Vec3(T p_x, T p_y, T p_z) : x(p_x), y(p_y), z(p_z) {};

        static inline Vec3 zero() {
            return {0};
        }
    };
}

#endif //PATHFINDER_VEC3_H
