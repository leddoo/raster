#pragma once

#include <lpp/core/basic.hpp>

#include <climits>


namespace lpp {

    template <typename T>
    constexpr Usize bits_per_value() {
        return CHAR_BIT*sizeof(T);
    }


    template <typename T>
    constexpr T mask_none() {
        return T(0);
    }

    template <typename T>
    constexpr T mask_all() {
        return T(-1);
    }

    template <typename T>
    constexpr T mask_all_equal(Bool value) {
        return T(value * T(-1));
    }


    template <typename T>
    constexpr T mask_only(Usize index) {
        if(index >= bits_per_value<T>()) {
            return T(0);
        }
        else {
            return T(T(1) << T(index));
        }
    }

    template <typename T>
    constexpr T mask_all_but(Usize index) {
        return T(~mask_only<T>(index));
    }


    // index: 0 => 000, 1 => 001, -1 => 111
    template <typename T>
    constexpr T mask_ending_at(Usize index) {
        if(index == 0) {
            return mask_none<T>();
        }
        else {
            return T(mask_only<T>(index) - T(1));
        }
    }

    // index: 0 => 111, 1 => 110, -1 => 000
    template <typename T>
    constexpr T mask_beginning_at(Usize index) {
        return T(~mask_ending_at<T>(index));
    }



    template <typename T>
    Usize leading_zeros(T value) {
        if(value == mask_none<T>()) {
            return bits_per_value<T>();
        }

        auto cursor = Usize(0);
        auto count  = Usize(0);
        auto width  = bits_per_value<T>()/2;

        // binary search: range is always top 2*width bits. that is: we always
        // choose the upper half, moving the value instead of the range when
        // choosing the lower half.
        while(width > 0) {
            // move cursor into middle of range.
            cursor += width;

            // check upper half.
            if((value >> cursor) == 0) {
                // no bits set => count "width" many zeros.
                count += width;
                // choose lower half by shifting it up.
                value <<= width;
            }

            width /= 2;
        }

        return count;
    }

    template <typename T>
    Bool is_power_of_2(T value) {
        if(value == T(0)) {
            return false;
        }
        else {
            return (value & (value - T(1))) == mask_none<T>();
        }
    }

    template <typename T>
    T next_power_of_2(T value) {
        if(value <= T(1)) {
            return T(1);
        }
        else {
            return mask_only<T>(bits_per_value<T>() - leading_zeros(value - 1));
        }
    }


    inline Usize aligned_ptr(Usize pointer, Usize alignment) {
        if(_not(is_power_of_2(alignment))) {
            throw "Alignment must be a power of 2.";
        }
        return (Usize(pointer) + (alignment - 1)) & ~(alignment - 1);
    }

    inline Addr aligned_ptr(Addr pointer, Usize alignment) {
        return Addr(aligned_ptr(Usize(pointer), alignment));
    }

}

