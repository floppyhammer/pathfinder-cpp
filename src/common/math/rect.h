#ifndef PATHFINDER_RECT_H
#define PATHFINDER_RECT_H

#include "vec2.h"

#undef min
#undef max

namespace Pathfinder {
    template<typename T>
    struct Rect {
        // A valid rect is when left <= right and top <= bottom.
        // A rect is initially invalid.
        T left = 1;
        T top = 1;
        T right = 0;
        T bottom = 0;

        // Invalid if not initialized.
        Rect() = default;

        // Valid if initialized.
        Rect(T p_left, T p_top, T p_right, T p_bottom)
                : left(p_left), top(p_top), right(p_right), bottom(p_bottom) {}

        Rect(Vec2 <T> p_left_top, Vec2 <T> p_right_bottom)
                : left(p_left_top.x), top(p_left_top.y), right(p_right_bottom.x), bottom(p_right_bottom.y) {}

        explicit Rect(T value[4])
                : left(value[0]), top(value[1]), right(value[2]), bottom(value[3]) {}

        template<typename U>
        explicit Rect(Rect<U> p_rect) {
            left = static_cast<T>(p_rect.left);
            top = static_cast<T>(p_rect.top);
            right = static_cast<T>(p_rect.right);
            bottom = static_cast<T>(p_rect.bottom);
        }

        template<typename U>
        inline Rect operator+(const Vec2 <U> &v) const {
            return {left + v.x, top + v.y, right + v.x, bottom + v.y};
        }

        template<typename U>
        inline Rect operator-(const Vec2 <U> &v) const {
            return {left - v.x, top - v.y, right - v.x, bottom - v.y};
        }

        template<typename U>
        inline Rect operator*(const Vec2 <U> &v) const {
            return {left * v.x, top * v.y, right * v.x, bottom * v.y};
        }

        template<typename U>
        inline void operator+=(const Vec2 <U> &v) {
            left += v.x;
            top += v.y;
            right += v.x;
            bottom += v.y;
        }

        template<typename U>
        inline void operator*=(const Vec2 <U> &v) {
            left *= v.x;
            top *= v.y;
            right *= v.x;
            bottom *= v.y;
        }

        template<typename U>
        inline bool operator==(const Rect<U> &r) const {
            return left == r.left && top == r.top && right == r.right && bottom == r.bottom;
        }

        friend std::ostream &operator<<(std::ostream &os, const Rect &r) {
            os << "Rect(" << r.left << ", " << r.top << ", " << r.right << ", " << r.bottom << ')';
            return os;
        }

        inline bool is_valid() const {
            return left <= right && top <= bottom;
        }

        inline T width() const {
            return right - left;
        }

        inline T height() const {
            return bottom - top;
        }

        inline Vec2 <T> origin() const {
            return {left, top};
        }

        inline Vec2 <T> upper_right() const {
            return {right, top};
        }

        inline Vec2 <T> lower_right() const {
            return {right, bottom};
        }

        inline Vec2 <T> lower_left() const {
            return {left, bottom};
        }

        inline Vec2 <T> size() const {
            return lower_right() - origin();
        }

        inline T area() const {
            return width() * height();
        }

        inline T min_x() const {
            return left;
        }

        inline T min_y() const {
            return top;
        }

        inline T max_x() const {
            return right;
        }

        inline T max_y() const {
            return bottom;
        }

        inline Vec2 <T> center() const {
            return Vec2<T>(left + right, top + bottom) * 0.5f;
        }

        inline Rect<float> to_f32() const {
            return {(float) left, (float) top, (float) right, (float) bottom};
        }

        inline Rect<int> to_i32() const {
            return {(int) round(left), (int) round(top), (int) round(right), (int) round(bottom)};
        }

        inline Rect<int> round_out() const {
            return {origin().floor(), lower_right().ceil()};
        }

        inline Rect<T> dilate(T amount) {
            return {origin() - amount, lower_right() + amount};
        }

        /// Check if intersects with other rect.
        inline bool intersects(const Rect &other) const {
            return !(left > other.right || right < other.left || top > other.bottom || bottom < other.top);
        }

        /// Return intersection rect. Return a invalid zero rect if no intersection.
        inline Rect intersection(const Rect &other) const {
            // If not intersected, return a zero rect.
            if (!intersects(other)) {
                return {};
            } else {
                return {origin().max(other.origin()), lower_right().min(other.lower_right())};
            }
        }

        inline Rect union_rect(const Rect &other) const {
            // This rect is invalid, return the other rect.
            if (!is_valid()) {
                return {other.origin(), other.lower_right()};
            }

            // The other rect is invalid, return this rect.
            if (!other.is_valid()) {
                return {origin(), lower_right()};
            }

            return {origin().min(other.origin()), lower_right().max(other.lower_right())};
        }

        // The containment check is a bit different for int and float.
        inline bool contains_point(const Vec2<float> &point) {
            // self.origin <= point && point <= self.lower_right
            return (left <= point.x && point.x <= right && top <= point.y && point.y <= bottom);
        }

        inline bool contains_point(const Vec2<int> &point) {
            // self.origin <= point && point <= self.lower_right - 1
            return (left <= point.x && point.x <= right - 1 && top <= point.y && point.y <= bottom - 1);
        }
    };

    inline void union_rect(Rect<float> &bounds, Vec2<float> new_point, bool first_point = false) {
        if (!bounds.is_valid() || first_point) {
            bounds = Rect<float>(new_point, new_point);
        } else {
            bounds = Rect<float>(bounds.origin().min(new_point), bounds.lower_right().max(new_point));
        }
    }
}

#endif //PATHFINDER_RECT_H
