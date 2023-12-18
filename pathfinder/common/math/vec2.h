#ifndef PATHFINDER_VEC2_H
#define PATHFINDER_VEC2_H

#include <algorithm>
#include <cmath>
#include <sstream>

#include "../logger.h"

#undef min
#undef max

namespace Pathfinder {

template <typename T>
struct Vec2 {
    T x = 0;
    T y = 0;

    Vec2() = default;

    explicit Vec2(T s) : x(s), y(s){};

    Vec2(T x, T y) : x(x), y(y){};

    Vec2<int32_t> floor() const {
        return {(int32_t)std::floor(x), (int32_t)std::floor(y)};
    }

    Vec2<int32_t> ceil() const {
        return {(int32_t)std::ceil(x), (int32_t)std::ceil(y)};
    }

    Vec2 abs() const {
        return {std::abs(x), std::abs(y)};
    }

    Vec2<float> to_f32() const {
        return {(float)x, (float)y};
    }

    Vec2<int32_t> to_i32() const {
        return {(int32_t)x, (int32_t)y};
    }

    Vec2 min(const Vec2 &other) const {
        return {std::min(x, other.x), std::min(y, other.y)};
    }

    Vec2 max(const Vec2 &other) const {
        return {std::max(x, other.x), std::max(y, other.y)};
    }

    T square_length() const {
        return x * x + y * y;
    }

    float length() const {
        return std::sqrt(square_length());
    }

    Vec2 normalize() const {
#ifdef PATHFINDER_DEBUG
        if (length() == 0) {
            Logger::error("Attempted to normalize a vector of zero length. This may indicate a bug in your code!",
                          "Vec2");
        }
#endif
        return *this / length();
    }

    Vec2 sqrt() const {
        return {std::sqrt(x), std::sqrt(y)};
    }

    bool is_zero() const {
        return x == 0 && y == 0;
    }

    T area() const {
        return x * y;
    }

    bool approx_eq(const Vec2 &other, float epsilon) const {
        return (*this - other).length() <= epsilon;
    }

    /// Swap y and x.
    Vec2 yx() const {
        return {y, x};
    }

    Vec2 operator+(const Vec2 &b) const {
        return {x + b.x, y + b.y};
    }

    Vec2 operator-() const {
        return {-x, -y};
    }

    Vec2 operator-(const Vec2 &b) const {
        return {x - b.x, y - b.y};
    }

    Vec2 operator*(const Vec2 &b) const {
        return {x * b.x, y * b.y};
    }

    Vec2 operator/(const Vec2 &b) const {
        return {x / b.x, y / b.y};
    }

    Vec2 operator+(T s) const {
        return {x + s, y + s};
    }

    Vec2 operator-(T s) const {
        return {x - s, y - s};
    }

    Vec2 operator/(T s) const {
        return {x / s, y / s};
    }

    Vec2 operator*(T s) const {
        return {x * s, y * s};
    }

    bool operator==(const Vec2 &b) const {
        return x == b.x && y == b.y;
    }

    // For being used as ordered key.
    bool operator<(const Vec2 &b) const {
        return x < b.x && y < b.y;
    }

    bool operator!=(const Vec2 &b) const {
        return x != b.x || y != b.y;
    }

    void operator+=(const Vec2 &b) {
        x += b.x;
        y += b.y;
    }

    void operator-=(const Vec2 &b) {
        x -= b.x;
        y -= b.y;
    }

    void operator*=(const Vec2 &b) {
        x *= b.x;
        y *= b.y;
    }

    friend std::ostream &operator<<(std::ostream &os, const Vec2 &v) {
        os << "(" << v.x << ", " << v.y << ')';
        return os;
    }

    std::string to_string() const {
        std::ostringstream string_stream;
        string_stream << *this;
        return string_stream.str();
    }
};

template <typename T>
Vec2<T> operator/(float s, Vec2<T> v) {
    return {s / v.x, s / v.y};
}

template <typename T>
Vec2<T> lerp(const Vec2<T> &a, const Vec2<T> &b, float t) {
    return {a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
}

typedef Vec2<int32_t> Vec2I;
typedef Vec2<float> Vec2F;

} // namespace Pathfinder

#endif // PATHFINDER_VEC2_H
