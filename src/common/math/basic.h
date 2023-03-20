#ifndef PATHFINDER_BASIC_MATH_H
#define PATHFINDER_BASIC_MATH_H

#include <cstdint>

namespace Pathfinder {

const float PI = 3.141592653589f;

/// Convert degree to radian.
inline float deg2rad(float x) {
    return x * (PI / 180.0f);
}

/// Convert radian to degree.
inline float rad2deg(float x) {
    return x * (180.0f / PI);
}

/// Get the upper 2^x of value v.
inline unsigned long upper_power_of_two(unsigned long v) {
    if (v == 0) {
        return 0;
    }

    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}

template <class T>
inline T clamp(T value, T min, T max) {
    return value < min ? min : (value > max ? max : value);
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline int alignup_i32(int32_t a, int32_t b) {
    return (a + b - 1) / b;
}

inline bool is_close(float a, float b, float tol) {
    return std::abs(a - b) < tol;
}

inline static uint64_t fnv_hash(const char* bytes, size_t size) {
    const unsigned int fnv_prime = 0x811C9DC5;
    uint64_t hash = 0;

    for (uint32_t i = 0; i < size; i++) {
        hash *= fnv_prime;
        hash ^= (bytes[i]);
    }

    return hash;
}

} // namespace Pathfinder

#endif // PATHFINDER_BASIC_MATH_H
