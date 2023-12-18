#ifndef PATHFINDER_VEC3_H
#define PATHFINDER_VEC3_H

#include "vec2.h"

namespace Pathfinder {

template <typename T>
struct Vec3 {
    T x = 0;
    T y = 0;
    T z = 0;

    Vec3() = default;

    explicit Vec3(T s) : x(s), y(s), z(s){};

    Vec3(T x, T y, T z) : x(x), y(y), z(z){};

    Vec3(Vec2<T> v, T z) : x(v.x), y(v.y), z(z){};

    Vec2<T> xy() const {
        return {x, y};
    }

    Vec3 operator*(T s) const {
        return {x * s, y * s, z * s};
    }

    bool operator==(const Vec3 &b) const {
        return x == b.x && y == b.y && z == b.z;
    }

    void operator+=(const Vec3 &b) {
        x += b.x;
        y += b.y;
        z += b.z;
    }
};

typedef Vec3<float> Vec3F;

} // namespace Pathfinder

#endif // PATHFINDER_VEC3_H
