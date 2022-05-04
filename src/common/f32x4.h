#ifndef PATHFINDER_F32X4_H
#define PATHFINDER_F32X4_H

#include "global_macros.h"
#include "logger.h"
#include "math/vec2.h"

#undef min
#undef max

#ifdef PATHFINDER_SIMD_ENABLED

#ifdef __ANDROID__
// A C/C++ header file that converts Intel SSE intrinsics to Arm/Aarch64 NEON intrinsics.
#include <sse2neon.h>
#else
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

namespace Pathfinder {
    /// Four 32-bit floats (SIMD).
    struct F32x4 {
        __m128 v = _mm_setzero_ps();

        F32x4() = default;

        explicit F32x4(__m128 p_v) : v(p_v) {}

        F32x4(Vec2<float> a, Vec2<float> b) {
            v = _mm_setr_ps(a.x, a.y, b.x, b.y);
        }

        F32x4(float x, float y, float z, float w) {
            v = _mm_setr_ps(x, y, z, w);
        }

        inline static F32x4 splat(float x) {
            return F32x4(_mm_set_ps1(x));
        }

        inline F32x4 min(const F32x4 &other) const {
            return F32x4(_mm_min_ps(v, other.v));
        }

        inline F32x4 max(const F32x4 &other) const {
            return F32x4(_mm_max_ps(v, other.v));
        }

        inline F32x4 clamp(const F32x4 &p_min, const F32x4 &p_max) const {
            return max(p_min).min(p_max);
        }

        inline F32x4 minus() const {
            return F32x4(_mm_xor_ps(v, _mm_set_ps1(-0.0f)));
        }

        inline F32x4 abs() const {
            return F32x4(_mm_andnot_ps(_mm_set_ps1(-0.0f), v));
        }

        inline __m128i to_i32() const {
            return _mm_cvtps_epi32(v);
        }

        // Extraction.
        // -----------------------------------------
        /// Extract an element.
        template<unsigned int i>
        inline float get() const {
            return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(i, i, i, i)));
        }

        inline Vec2<float> xy() const {
            return {get<0>(), get<1>()};
        }

        inline Vec2<float> zw() const {
            return zwxy().xy();
        }

        inline F32x4 zwzw() const {
            // Method A.
//        return F32x4(_mm_shuffle_ps(v, v, 238));

            // Method B.
            return F32x4(_mm_movehl_ps(v, v));
        }

        inline F32x4 zwxy() const {
            // Fix the memory alignment issue on 32-bit machines,
            // which might be caused by using std::vector to hold Segments.
            // Check if the memory address is 16-byte aligned.
            if (((intptr_t)&v & 0xF) != 0) {
                //Logger::error("__m128 memory is not 16-byte aligned!", "SIMD");
                auto aligned = v;
                return F32x4(_mm_shuffle_ps(aligned, aligned, 78));
            }

            // 78 = _MM_SHUFFLE(1, 0, 3, 2)
            return F32x4(_mm_shuffle_ps(v, v, 78));
        }

        inline F32x4 xyxy() const {
            return F32x4(_mm_movelh_ps(v, v));
        }
        // -----------------------------------------

        // Concatenation.
        // -----------------------------------------
        inline F32x4 concat_xy_xy(const F32x4 &other) const {
            // Method A.
//        auto a = _mm_castps_pd(v);
//        auto b = _mm_castps_pd(other.v);
//        auto result = _mm_unpacklo_pd(a, b);
//        return F32x4(_mm_castpd_ps(result));

            // Method B.
            return F32x4(_mm_movelh_ps(v, other.v));
        }

        inline F32x4 concat_xy_zw(const F32x4 &other) const {
            auto a = _mm_castps_pd(v);
            auto b = _mm_castps_pd(other.v);
            auto result = _mm_shuffle_pd(a, b, 0b10);
            return F32x4(_mm_castpd_ps(result));
        }

        inline F32x4 concat_zw_zw(const F32x4 &other) const {
            // Method A.
//        auto a = _mm_castps_pd(v);
//        auto b = _mm_castps_pd(other.v);
//        auto result = _mm_unpackhi_pd(a, b);
//        return F32x4(_mm_castpd_ps(result));

            // Method B.
            return F32x4(_mm_movehl_ps(other.v, v));
        }
        // -----------------------------------------

        inline F32x4 operator+(const F32x4 &b) const {
            return F32x4(_mm_add_ps(v, b.v));
        }

        inline F32x4 operator-(const F32x4 &b) const {
            return F32x4(_mm_sub_ps(v, b.v));
        }

        inline F32x4 operator*(const F32x4 &b) const {
            return F32x4(_mm_mul_ps(v, b.v));
        }

        inline F32x4 operator/(const F32x4 &b) const {
            return F32x4(_mm_div_ps(v, b.v));
        }

        inline void operator+=(const F32x4 &b) {
            *this = *this + b;
        }

        inline void operator-=(const F32x4 &b) {
            *this = *this - b;
        }
    };
}

#else

namespace Pathfinder {
    /// Four 32-bit floats (Scalar, i.e. no SIMD).
    struct F32x4 {
        float v[4] = {0};

        F32x4(Vec2<float> a, Vec2<float> b) {
            v[0] = a.x;
            v[1] = a.y;
            v[2] = b.x;
            v[3] = b.y;
        }

        F32x4(float x, float y, float z, float w) {
            v[0] = x;
            v[1] = y;
            v[2] = z;
            v[3] = w;
        }

        inline static F32x4 splat(float x) {
            return {x, x, x, x};
        }

        inline F32x4 min(const F32x4 &other) const {
            return {
                    std::min(v[0], other.v[0]),
                    std::min(v[1], other.v[1]),
                    std::min(v[2], other.v[2]),
                    std::min(v[3], other.v[3])
            };
        }

        inline F32x4 max(const F32x4 &other) const {
            return {
                    std::max(v[0], other.v[0]),
                    std::max(v[1], other.v[1]),
                    std::max(v[2], other.v[2]),
                    std::max(v[3], other.v[3])
            };
        }

        inline F32x4 clamp(const F32x4 &p_min, const F32x4 &p_max) const {
            return max(p_min).min(p_max);
        }

        inline F32x4 minus() const {
            return {-v[0], -v[1], -v[2], -v[3]};
        }

        inline F32x4 abs() const {
            return {
                    std::abs(v[0]),
                    std::abs(v[1]),
                    std::abs(v[2]),
                    std::abs(v[3])
            };
        }

        // Extraction.
        // -----------------------------------------
        /// Extract an element.
        template<unsigned int i>
        inline float get() const {
            return v[i];
        }

        inline Vec2<float> xy() const {
            return {v[0], v[1]};
        }

        inline Vec2<float> zw() const {
            return {v[2], v[3]};
        }

        inline F32x4 zwzw() const {
            return {v[2], v[3], v[2], v[3]};
        }

        inline F32x4 zwxy() const {
            return {v[2], v[3], v[0], v[1]};
        }

        inline F32x4 xyxy() const {
            return {v[0], v[1], v[0], v[1]};
        }
        // -----------------------------------------

        // Concatenation.
        // -----------------------------------------
        inline F32x4 concat_xy_xy(const F32x4 &other) const {
            return {v[0], v[1], other.v[0], other.v[1]};
        }

        inline F32x4 concat_xy_zw(const F32x4 &other) const {
            return {v[0], v[1], other.v[2], other.v[3]};
        }

        inline F32x4 concat_zw_zw(const F32x4 &other) const {
            return {v[2], v[3], other.v[2], other.v[3]};
        }
        // -----------------------------------------

        inline F32x4 operator+(const F32x4 &b) const {
            return {v[0] + b.v[0], v[1] + b.v[1], v[2] + b.v[2], v[3] + b.v[3]};
        }

        inline F32x4 operator-(const F32x4 &b) const {
            return {v[0] - b.v[0], v[1] - b.v[1], v[2] - b.v[2], v[3] - b.v[3]};
        }

        inline F32x4 operator*(const F32x4 &b) const {
            return {v[0] * b.v[0], v[1] * b.v[1], v[2] * b.v[2], v[3] * b.v[3]};
        }

        inline F32x4 operator/(const F32x4 &b) const {
            return {v[0] / b.v[0], v[1] / b.v[1], v[2] / b.v[2], v[3] / b.v[3]};
        }
    };
}

#endif //PATHFINDER_SIMD_ENABLED

#endif //PATHFINDER_F32X4_H
