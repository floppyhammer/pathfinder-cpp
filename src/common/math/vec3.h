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

    Vec3(T p_x, T p_y, T p_z) : x(p_x), y(p_y), z(p_z){};

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
