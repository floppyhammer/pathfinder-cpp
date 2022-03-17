//
// Created by chy on 7/5/2021.
//

#ifndef PATHFINDER_VEC2_H
#define PATHFINDER_VEC2_H

#include <iostream>
#include <cmath>

#undef min
#undef max

namespace Pathfinder {
    template<typename T>
    struct Vec2 {
        T x = 0;
        T y = 0;

        Vec2() = default;

        explicit Vec2(T p_s) : x(p_s), y(p_s) {};

        Vec2(T p_x, T p_y) : x(p_x), y(p_y) {};

        static inline Vec2 zero() {
            return {0};
        }

        inline Vec2<int> floor() const {
            return Vec2<int>{(int) std::floor(x), (int) std::floor(y)};
        }

        inline Vec2<int> ceil() const {
            return Vec2<int>{(int) std::ceil(x), (int) std::ceil(y)};
        }

        inline Vec2 abs() const {
            return Vec2{std::abs(x), std::abs(y)};
        }

        inline Vec2<float> to_float() const {
            return Vec2<float>{(float) x, (float) y};
        }

        inline Vec2 min(const Vec2 &other) const {
            return Vec2(std::min(x, other.x), std::min(y, other.y));
        }

        inline Vec2 max(const Vec2 &other) const {
            return Vec2(std::max(x, other.x), std::max(y, other.y));
        }

        inline float square_length() const {
            return x * x + y * y;
        }

        inline T length() const {
            return std::sqrt(square_length());
        }

        inline Vec2 normalize() const {
            return Vec2(x, y) / length();
        }

        inline Vec2 sqrt() const {
            return Vec2(std::sqrt(x), std::sqrt(y));
        }

        inline bool is_zero() const {
            return x == 0 && y == 0;
        }

        inline T area() {
            return x * y;
        }

        inline bool is_close(Vec2 other, float tol) const {
            return (*this - other).length() < tol;
        }

        inline Vec2<float> to_f32() const {
            return Vec2<float>(x, y);
        }

        /// Swap y and x.
        inline Vec2 yx() const {
            return Vec2(y, x);
        }

        inline Vec2 operator+(const Vec2 &b) const {
            return Vec2{x + b.x, y + b.y};
        }

        inline Vec2 operator-() const {
            return Vec2{-x, -y};
        }

        inline Vec2 operator-(const Vec2 &b) const {
            return Vec2{x - b.x, y - b.y};
        }

        inline Vec2 operator*(const Vec2 &b) const {
            return Vec2{x * b.x, y * b.y};
        }

        inline Vec2 operator/(const Vec2 &b) const {
            return Vec2{x / b.x, y / b.y};
        }

        inline Vec2 operator+(float s) const {
            return Vec2{x + s, y + s};
        }

        inline Vec2 operator-(float s) const {
            return Vec2{x - s, y - s};
        }

        inline Vec2 operator/(float s) const {
            return Vec2{x / s, y / s};
        }

        inline Vec2 operator*(float s) const {
            return Vec2{x * s, y * s};
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
    };

    template<typename T>
    inline Vec2<T> operator/(float s, Vec2<T> v) {
        return Vec2<T>{s / v.x, s / v.y};
    }

    template<typename T>
    inline Vec2<T> lerp(const Vec2<T> &a, const Vec2<T> &b, float t) {
        return Vec2<T>{a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
    }
}

#endif //PATHFINDER_VEC2_H
