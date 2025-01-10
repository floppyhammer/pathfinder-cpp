#ifndef PATHFINDER_RECT_H
#define PATHFINDER_RECT_H

#include "vec2.h"

#undef min
#undef max

namespace Pathfinder {

template <typename T>
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
    Rect(T left, T top, T right, T bottom) : left(left), top(top), right(right), bottom(bottom) {}

    Rect(Vec2<T> left_top, Vec2<T> right_bottom)
        : left(left_top.x), top(left_top.y), right(right_bottom.x), bottom(right_bottom.y) {}

    explicit Rect(T values[4]) : left(values[0]), top(values[1]), right(values[2]), bottom(values[3]) {}

    template <typename U>
    explicit Rect(const Rect<U> &other) {
        left = static_cast<T>(other.left);
        top = static_cast<T>(other.top);
        right = static_cast<T>(other.right);
        bottom = static_cast<T>(other.bottom);
    }

    template <typename U>
    Rect operator+(const Vec2<U> &v) const {
        return {left + v.x, top + v.y, right + v.x, bottom + v.y};
    }

    template <typename U>
    Rect operator-(const Vec2<U> &v) const {
        return {left - v.x, top - v.y, right - v.x, bottom - v.y};
    }

    template <typename U>
    Rect operator*(const Vec2<U> &v) const {
        return {left * v.x, top * v.y, right * v.x, bottom * v.y};
    }

    template <typename U>
    Rect operator*(U v) const {
        return {left * v, top * v, right * v, bottom * v};
    }

    template <typename U>
    void operator+=(const Vec2<U> &v) {
        left += v.x;
        top += v.y;
        right += v.x;
        bottom += v.y;
    }

    template <typename U>
    void operator*=(const Vec2<U> &v) {
        left *= v.x;
        top *= v.y;
        right *= v.x;
        bottom *= v.y;
    }

    template <typename U>
    bool operator==(const Rect<U> &rhs) const {
        return left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom;
    }

    friend std::ostream &operator<<(std::ostream &os, const Rect &r) {
        os << "Rect(" << r.left << ", " << r.top << ", " << r.right << ", " << r.bottom << ')';
        return os;
    }

    bool is_valid() const {
        return left <= right && top <= bottom;
    }

    T width() const {
        return right - left;
    }

    T height() const {
        return bottom - top;
    }

    Vec2<T> origin() const {
        return {left, top};
    }

    Vec2<T> upper_right() const {
        return {right, top};
    }

    Vec2<T> lower_right() const {
        return {right, bottom};
    }

    Vec2<T> lower_left() const {
        return {left, bottom};
    }

    Vec2<T> size() const {
        return lower_right() - origin();
    }

    T area() const {
        return width() * height();
    }

    T min_x() const {
        return left;
    }

    T min_y() const {
        return top;
    }

    T max_x() const {
        return right;
    }

    T max_y() const {
        return bottom;
    }

    Vec2<T> center() const {
        return Vec2<T>(left + right, top + bottom) * 0.5f;
    }

    Rect<T> contract(Vec2<T> amount) {
        return {origin() + amount, lower_right() - amount};
    }

    Rect<float> to_f32() const {
        return {(float)left, (float)top, (float)right, (float)bottom};
    }

    Rect<int32_t> to_i32() const {
        return {int32_t(round(left)), int32_t(round(top)), int32_t(round(right)), int32_t(round(bottom))};
    }

    Rect<int32_t> round_out() const {
        return {origin().floor(), lower_right().ceil()};
    }

    Rect<T> dilate(T amount) {
        return {origin() - amount, lower_right() + amount};
    }

    /// Check if intersects with other rect.
    bool intersects(const Rect &other) const {
        return !(left > other.right || right < other.left || top > other.bottom || bottom < other.top);
    }

    /// Return intersection rect. Return an invalid rect if no intersection.
    Rect intersection(const Rect &other) const {
        // If not intersected, return a zero rect.
        if (!intersects(other)) {
            return {};
        }

        return {origin().max(other.origin()), lower_right().min(other.lower_right())};
    }

    Rect union_rect(const Rect &other) const {
        // This rect is invalid, return the other rect.
        if (!is_valid()) {
            return other;
        }

        // The other rect is invalid, return this rect.
        if (!other.is_valid()) {
            return *this;
        }

        return {origin().min(other.origin()), lower_right().max(other.lower_right())};
    }

    // The containment check is a bit different for int and float.
    bool contains_point(const Vec2F &point) {
        // self.origin <= point && point <= self.lower_right
        return (left <= point.x && point.x <= right && top <= point.y && point.y <= bottom);
    }

    bool contains_point(const Vec2I &point) {
        // self.origin <= point && point <= self.lower_right - 1
        return (left <= point.x && point.x <= right - 1 && top <= point.y && point.y <= bottom - 1);
    }
};

inline void union_rect(Rect<float> &bounds, Vec2F new_point, bool first_point = false) {
    if (!bounds.is_valid() || first_point) {
        bounds = Rect<float>(new_point, new_point);
    } else {
        bounds = Rect<float>(bounds.origin().min(new_point), bounds.lower_right().max(new_point));
    }
}

// Try not to use other extended types unless the function `contains_point` is taken care of.
typedef Rect<int32_t> RectI;
typedef Rect<float> RectF;

} // namespace Pathfinder

#endif // PATHFINDER_RECT_H
