#ifndef PATHFINDER_VEC2_H
#define PATHFINDER_VEC2_H

#include <cmath>
#include <iostream>
#include <sstream>

#undef min
#undef max

namespace Pathfinder {

template <typename T>
struct Vec2 {
    T x = 0;
    T y = 0;

    Vec2() = default;

    explicit Vec2(T s) : x(s), y(s){};

    Vec2(T _x, T _y) : x(_x), y(_y){};

    inline Vec2<int32_t> floor() const {
        return {(int32_t)std::floor(x), (int32_t)std::floor(y)};
    }

    inline Vec2<int32_t> ceil() const {
        return {(int32_t)std::ceil(x), (int32_t)std::ceil(y)};
    }

    inline Vec2 abs() const {
        return {std::abs(x), std::abs(y)};
    }

    inline Vec2<float> to_f32() const {
        return {(float)x, (float)y};
    }

    inline Vec2<int32_t> to_i32() const {
        return {(int32_t)x, (int32_t)y};
    }

    inline Vec2 min(const Vec2 &other) const {
        return {std::min(x, other.x), std::min(y, other.y)};
    }

    inline Vec2 max(const Vec2 &other) const {
        return {std::max(x, other.x), std::max(y, other.y)};
    }

    inline T square_length() const {
        return x * x + y * y;
    }

    inline float length() const {
        return std::sqrt(square_length());
    }

    inline Vec2 normalize() const {
        return *this / length();
    }

    inline Vec2 sqrt() const {
        return {std::sqrt(x), std::sqrt(y)};
    }

    inline bool is_zero() const {
        return x == 0 && y == 0;
    }

    inline T area() const {
        return x * y;
    }

    inline bool is_close(const Vec2 &other, float tol) const {
        return (*this - other).length() < tol;
    }

    /// Swap y and x.
    inline Vec2 yx() const {
        return {y, x};
    }

    inline Vec2 operator+(const Vec2 &b) const {
        return {x + b.x, y + b.y};
    }

    inline Vec2 operator-() const {
        return {-x, -y};
    }

    inline Vec2 operator-(const Vec2 &b) const {
        return {x - b.x, y - b.y};
    }

    inline Vec2 operator*(const Vec2 &b) const {
        return {x * b.x, y * b.y};
    }

    inline Vec2 operator/(const Vec2 &b) const {
        return {x / b.x, y / b.y};
    }

    inline Vec2 operator+(T s) const {
        return {x + s, y + s};
    }

    inline Vec2 operator-(T s) const {
        return {x - s, y - s};
    }

    inline Vec2 operator/(T s) const {
        return {x / s, y / s};
    }

    inline Vec2 operator*(T s) const {
        return {x * s, y * s};
    }

    inline bool operator==(const Vec2 &b) const {
        return (x == b.x && y == b.y);
    }

    inline bool operator!=(const Vec2 &b) const {
        return (x != b.x || y != b.y);
    }

    inline void operator+=(const Vec2 &b) {
        x += b.x;
        y += b.y;
    }

    inline void operator-=(const Vec2 &b) {
        x -= b.x;
        y -= b.y;
    }

    inline void operator*=(const Vec2 &b) {
        x *= b.x;
        y *= b.y;
    }

    friend std::ostream &operator<<(std::ostream &os, const Vec2 &v) {
        os << "Vec2(" << v.x << ", " << v.y << ')';
        return os;
    }

    inline std::string to_string() const {
        std::ostringstream string_stream;
        string_stream << this;
        return string_stream.str();
    }
};

template <typename T>
inline Vec2<T> operator/(float s, Vec2<T> v) {
    return {s / v.x, s / v.y};
}

template <typename T>
inline Vec2<T> lerp(const Vec2<T> &a, const Vec2<T> &b, float t) {
    return {a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
}

typedef Vec2<int32_t> Vec2I;
typedef Vec2<float> Vec2F;

} // namespace Pathfinder

#endif // PATHFINDER_VEC2_H
