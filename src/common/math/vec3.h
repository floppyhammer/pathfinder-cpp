#ifndef PATHFINDER_VEC3_H
#define PATHFINDER_VEC3_H

namespace Pathfinder {

template <typename T>
struct Vec3 {
    T x = 0;
    T y = 0;
    T z = 0;

    Vec3() = default;

    explicit Vec3(T s) : x(s), y(s), z(s){};

    Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z){};

    Vec3(Vec2<T> _vec2, T _z) : x(_vec2.x), y(_vec2.y), z(_z){};

    Vec2<T> xy() const {
        return {x, y};
    }

    inline Vec3 operator*(float s) const {
        return {x * s, y * s, z * s};
    }

    inline void operator+=(const Vec3 &b) {
        x += b.x;
        y += b.y;
        z += b.z;
    }
};

typedef Vec3<float> Vec3F;

} // namespace Pathfinder

#endif // PATHFINDER_VEC3_H
