#ifndef PATHFINDER_I32X4_H
#define PATHFINDER_I32X4_H

#include <cstdint>

#include "../config.h"

#ifdef __ANDROID__
    // A C/C++ header file that converts Intel SSE intrinsics to Arm/Aarch64 NEON intrinsics.
    #include <sse2neon.h>
#else
    #include <emmintrin.h>
    #include <xmmintrin.h>
// #include <immintrin.h> // Required by AVX2, which we won't be using due its absence on ARM.
#endif

namespace Pathfinder {

/// Four 32-bit ints (SIMD).
struct I32x4 {
    __m128i v = _mm_setzero_si128();

    I32x4() = default;

    explicit I32x4(__m128i _v) : v(_v) {}

    I32x4(int32_t x, int32_t y, int32_t z, int32_t w) {
        v = _mm_setr_epi32(x, y, z, w);
    }

    inline static I32x4 splat(int32_t x) {
        return I32x4(_mm_set1_epi32(x));
    }

    inline I32x4 shift_l(int32_t count) const {
        // Same as _mm_sllv_epi32(v, _mm_set1_epi32(count)), but that requires AVX2.
        // Cf. https://stackoverflow.com/questions/14731442/am-i-using-mm-srl-epi32-wrong
        return I32x4(_mm_sll_epi32(v, _mm_set_epi32(0, 0, 0, count)));
    }

    inline I32x4 shift_r(int32_t count) const {
        // Same as _mm_srlv_epi32(v, _mm_set1_epi32(count)), but that requires AVX2.
        // Cf. https://stackoverflow.com/questions/14731442/am-i-using-mm-srl-epi32-wrong
        return I32x4(_mm_srl_epi32(v, _mm_set_epi32(0, 0, 0, count)));
    }

    inline I32x4 operator+(const I32x4 &b) const {
        return I32x4(_mm_add_epi32(v, b.v));
    }

    inline I32x4 operator-(const I32x4 &b) const {
        return I32x4(_mm_sub_epi32(v, b.v));
    }

    inline I32x4 operator*(const I32x4 &b) const {
        // Multiply 2 and 0.
        __m128i tmp1 = _mm_mul_epu32(v, b.v);
        // Multiply 3 and 1.
        __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(v, 4), _mm_srli_si128(b.v, 4));
        // Shuffle results to [63..0] and pack.
        // 8 = _MM_SHUFFLE(0, 0, 2, 0)
        return I32x4(_mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, 8), _mm_shuffle_epi32(tmp2, 8)));
    }

    inline void operator+=(const I32x4 &b) {
        *this = *this + b;
    }

    inline void operator-=(const I32x4 &b) {
        *this = *this - b;
    }
};

} // namespace Pathfinder

#endif // PATHFINDER_I32X4_H
