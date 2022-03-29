#pragma once

#include <lpp/core/basic.hpp>
#include <lpp/core/basic_type_info.hpp>
#include <lpp/common/math.hpp>
#include <lpp/common/bit_math.hpp>
#include <lpp/memory/list.hpp>
#include <lpp/memory/arena.hpp>
#include <lpp/memory/buffer_utils.hpp>

#include "array.hpp"
#include "vector.hpp"

using namespace lpp;


constexpr S32 s32_max =  2147483647;
constexpr S32 s32_min = -2147483647 - 1;

namespace lpp {
    static F32 floor(F32 value) {
        if(value >= 0) {
            return F32(S32(value));
        }
        else {
            auto as_s32 = S32(value);
            if(value == F32(as_s32)) {
                return value;
            }
            else {
                return F32(as_s32 - 1);
            }
        }
    }

    inline F32 ceil(F32 value) {
        return -floor(-value);
    }

    inline S32 floor_to_s32(F32 value) {
        return S32(floor(value));
    }

    inline F32 round_away_from_0(F32 value) {
        if(value >= 0) {
            return ceil(value);
        }
        else {
            return floor(value);
        }
    }


    template <typename T>
    T abs(T value) { return (value >= T(0)) ? value : -value; }

    template <typename T>
    T squared(T value) { return value*value; }

    template <typename T>
    T sign(T value) {
        if     (value < T(0)) { return T(-1); }
        else if(value > T(0)) { return T( 1); }
        else                  { return T( 0); }
    }

    lpp_def_vector_proc_1_simple(abs)
    lpp_def_vector_proc_1_simple(squared)
    lpp_def_vector_proc_1_simple(sign)
    lpp_def_vector_proc_1(floor, LPP_PASS(std::floor(a[i])))

    lpp_def_vector_proc_2_simple(min)
    lpp_def_vector_proc_2_simple(max)
    lpp_def_vector_proc_2_simple(at_least)
    lpp_def_vector_proc_2_simple(at_most)



    inline F32 dot(V2f a, V2f b) {
        return a.x()*b.x() + a.y()*b.y();
    }

    inline F32 length_squared(V2f v) {
        return dot(v, v);
    }

    F32 length(V2f v);

    inline V2f normalized(V2f v) {
        return 1.0f/(length(v)) * v;
    }

    inline V2f rotated_acw(V2f v) {
        return V2f({ -v.y(), v.x() });
    }

    inline V2f rotated_cw(V2f v) {
        return V2f({ v.y(), -v.x() });
    }



    template <typename T, typename Leq>
    Void bubble_sort_right_to_left(Ref<List<T>> list, Leq leq) {
        if(list.length < 2) {
            return;
        }

        const auto list_end = list.length;
        auto sorted_end = Usize(0);

        while(sorted_end < list_end) {
            auto new_sorted_end = list_end;

            for(Usize i = list_end - 1; i > sorted_end; i -= 1) {
                auto& a = list[i - 1];
                auto& b = list[i];
                if(!leq(a, b)) {
                    std::swap(a, b);
                    new_sorted_end = i;
                }
            }

            sorted_end = new_sorted_end;
        }
    }


    Void fill_copy_bytes(Addr begin, Addr end, Addr filled_until);

}

namespace raster {


    template <typename T, Usize degree>
    struct Polynomial : Array<T, degree + 1> {
        using Array<T, degree + 1>::Array;
    };

    template <typename T, Usize degree>
    struct Bezier : Array<T, degree + 1> {
        using Array<T, degree + 1>::Array;
    };

    template <typename T>
    struct Segment : Array<T, 2> {
        using Array<T, 2>::Array;

        Ref<      V2f> p0()       { return this->values()[0]; }
        Ref<const V2f> p0() const { return this->values()[0]; }

        Ref<      V2f> p1()       { return this->values()[1]; }
        Ref<const V2f> p1() const { return this->values()[1]; }
    };



    struct Color_Rgba {
        U32 value;

        explicit Color_Rgba(U32 value) : value(value) {}
        explicit Color_Rgba(U8 r, U8 g, U8 b, U8 a);

        static Color_Rgba pack_255(F32 r, F32 g, F32 b, F32 a);
        static Color_Rgba pack_255(V4f color);
        static Color_Rgba pack(F32 r, F32 g, F32 b, F32 a);
        static Color_Rgba pack(V4f color);

        V4f unpack_255() const;
        V4f unpack()   const;
    };


    struct Color_Bgra {
        U32 value;

        explicit Color_Bgra(U32 value) : value(value) {}
        explicit Color_Bgra(U8 r, U8 g, U8 b, U8 a);

        static Color_Bgra pack_255(F32 r, F32 g, F32 b, F32 a);
        static Color_Bgra pack_255(V4f color);
        static Color_Bgra pack(F32 r, F32 g, F32 b, F32 a);
        static Color_Bgra pack(V4f color);

        V4f unpack_255() const;
        V4f unpack() const;
    };

    template <typename Sample>
    struct Image {
        Ptr<Sample> samples;
        V2u lengths;
        U16 sample_count;


        static Image create(U32 width, U32 height, U16 sample_count);

        Ptr<const Sample> get_first_sample(U32 x, U32 y) const;
        Ptr<      Sample> get_first_sample(U32 x, U32 y);
    };

}



namespace raster {

    template <typename Sample>
    Image<Sample> Image<Sample>::create(U32 width, U32 height, U16 sample_count) {
        auto image = Image();
        image.samples      = buffer::allocate<Sample>(width*height*sample_count);
        image.lengths      = V2u({ width, height });
        image.sample_count = sample_count;
        return image;
    }


    template <typename Sample>
    Ptr<const Sample> Image<Sample>::get_first_sample(U32 x, U32 y) const {
        return &this->samples[this->sample_count * (y*this->lengths.x() + x)];
    }

    template <typename Sample>
    Ptr<Sample> Image<Sample>::get_first_sample(U32 x, U32 y) {
        return &this->samples[this->sample_count * (y*this->lengths.x() + x)];
    }


}

