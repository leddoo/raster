#pragma once

#include "common.hpp"

#include <intrin.h>

namespace raster {

    struct U8x16 {
        __m128i value;

        U8x16() : value(_mm_setzero_si128()) {}
        explicit U8x16(U8 value) : value(_mm_set1_epi8(S8(value))) {}
        U8x16(__m128i value) : value(value) {}


        static U8x16 unpack_bits(U16 bits) {
            auto bits_x8 = _mm_set1_epi16(S16(bits));

            // duplicate each u8 in bits 8 times.
            auto selector = _mm_set_epi8(
                1, 1, 1, 1, 1, 1, 1, 1,
                0, 0, 0, 0, 0, 0, 0, 0
            );
            auto duplicated_u8s = _mm_shuffle_epi8(bits_x8, selector);

            // the bit selector for each duplicated u8.
            auto bit_mask = _mm_set_epi8(
                S8(U8(0x80)), 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
                S8(U8(0x80)), 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
            );

            // for each duplicated u8: check if the appropriate bit is set.
            auto u8s = _mm_cmpeq_epi8(
                _mm_and_si128(bit_mask, duplicated_u8s),
                bit_mask
            );

            return u8s;
        }


        U8x16 load()            { return _mm_loadu_si128(&this->value); }
        Void store(U8x16 value) { _mm_storeu_si128(&this->value, value.value); }


        U16 high_bits_to_mask() const {
            return U16(_mm_movemask_epi8(this->value));
        }
    };

    inline U8x16 operator+(U8x16 a, U8x16 b)  { return _mm_add_epi8(a.value, b.value); }
    inline U8x16 operator&(U8x16 a, U8x16 b)  { return _mm_and_si128(a.value, b.value); }
    inline U8x16 operator==(U8x16 a, U8x16 b) { return _mm_cmpeq_epi8(a.value, b.value); }

    inline U8x16 masked_add(U8x16 a, U8x16 b, U16 mask) {
        return a + (b & U8x16::unpack_bits(mask));
    }



    struct S16x8 {
        __m128i value;

        S16x8() : value(_mm_setzero_si128()) {}
        explicit S16x8(S16 value) : value(_mm_set1_epi16(value)) {}
        S16x8(__m128i value) : value(value) {}


        S16x8 load()            { return _mm_loadu_si128(&this->value); }
        Void store(S16x8 value) { _mm_storeu_si128(&this->value, value.value); }
    };


    struct S32x4 {
        __m128i value;

        S32x4() : value(_mm_setzero_si128()) {}
        explicit S32x4(S32 value) : value(_mm_set1_epi32(value)) {}
        S32x4(__m128i value) : value(value) {}


        S32x4 load()            { return _mm_loadu_si128(&this->value); }
        Void store(S32x4 value) { _mm_storeu_si128(&this->value, value.value); }
    };

    struct U32x4 {
        __m128i value;

        U32x4() : value(_mm_setzero_si128()) {}
        explicit U32x4(U32 value) : value(_mm_set1_epi32(S32(value))) {}
        U32x4(__m128i value) : value(value) {}


        static U32x4 unpack_bits(U32 bits) {
            auto bits_x4 = _mm_set1_epi32(S32(bits));

            auto bit_mask = _mm_set_epi32(0x08, 0x04, 0x02, 0x01);

            auto u32s = _mm_cmpeq_epi32(
                _mm_and_si128(bit_mask, bits_x4),
                bit_mask
            );

            return u32s;
        }



        U32x4 load()            { return _mm_loadu_si128(&this->value); }
        Void store(U32x4 value) { _mm_storeu_si128(&this->value, value.value); }
    };

    inline U32x4 operator&(U32x4 a, U32x4 b)  { return _mm_and_si128(a.value, b.value); }
    inline U32x4 operator|(U32x4 a, U32x4 b)  { return _mm_or_si128(a.value, b.value); }
    inline U32x4 operator~(U32x4 a) { return _mm_xor_si128(a.value, _mm_set1_epi32(-1)); }



    struct F32x4 {
        __m128 value;

        F32x4() : value(_mm_setzero_ps()) {}
        explicit F32x4(F32 value) : value(_mm_set1_ps(value)) {}
        F32x4(F32 a0, F32 a1, F32 a2, F32 a3) : value(_mm_set_ps(a3, a2, a1, a0)) {}
        explicit F32x4(V4f value) : value(_mm_loadu_ps(value.values())) {}
        F32x4(__m128 value) : value(value) {}


        F32x4 load()            { return _mm_loadu_ps(reinterpret_cast<Ptr<F32>>(&this->value)); }
        Void store(F32x4 value) { _mm_storeu_ps(reinterpret_cast<Ptr<F32>>(&this->value), value.value); }
    };

    inline F32x4 operator+(F32x4 a, F32x4 b) { return _mm_add_ps(a.value, b.value); }
    inline F32x4 operator-(F32x4 a, F32x4 b) { return _mm_sub_ps(a.value, b.value); }
    inline F32x4 operator*(F32x4 a, F32x4 b) { return _mm_mul_ps(a.value, b.value); }
    inline F32x4 operator/(F32x4 a, F32x4 b) { return _mm_div_ps(a.value, b.value); }


    inline F32x4 shuffle_rgba_to_bgra(F32x4 a) {
        return _mm_shuffle_ps(a.value, a.value, (2 << 0) | (1 << 2) | (0 << 4) | (3 << 6));
    }

    inline F32x4 shuffle_bgra_to_rgba(F32x4 a) {
        return _mm_shuffle_ps(a.value, a.value, (2 << 0) | (1 << 2) | (0 << 4) | (3 << 6));
    }


    inline U8x16 interpret_as_u8s(U32x4 a) { return a.value; }

    inline S32x4 to_s32s(F32x4 a) { return _mm_cvtps_epi32(a.value); }
    inline F32x4 to_f32s(S32x4 a) { return _mm_cvtepi32_ps(a.value); }

    inline V4f to_v4f(F32x4 a) { return reinterpret_cast<Ref<V4f>>(a); }

    inline S16x8 pack_with_signed_saturation(S32x4 a, S32x4 b = S32x4()) { return _mm_packs_epi32(a.value, b.value); }
    inline U8x16 pack_with_unsigned_saturation(S16x8 a, S16x8 b = S16x8()) { return _mm_packus_epi16(a.value, b.value); }

    inline S16x8 unpack_low(U8x16 a, U8x16 b = U8x16()) { return _mm_unpacklo_epi8(a.value, b.value); }
    inline S32x4 unpack_low(S16x8 a, S16x8 b = S16x8()) { return _mm_unpacklo_epi16(a.value, b.value); }
}


namespace lpp {
    template <>
    inline raster::F32x4 min(raster::F32x4 a, raster::F32x4 b) { return _mm_min_ps(a.value, b.value); }

    template <>
    inline raster::F32x4 max(raster::F32x4 a, raster::F32x4 b) { return _mm_max_ps(a.value, b.value); }
}

