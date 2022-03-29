#define _CRT_SECURE_NO_WARNINGS
#include "common.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push)
    #pragma warning(disable: 4365)
    #include "stb_image_write.h"
#pragma warning(pop)

using namespace raster;


#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <cassert>


proto::Allocator_Record malloc_record = {
    [](Addr, Usize size, Usize alignment) -> Addr {
        LPP_UNUSED(alignment);
        return Addr(std::malloc(size));
    },
    [](Addr, Addr allocation) {
        std::free(allocation);
    },
};
Allocator malloc_allocator = { &malloc_record };

namespace lpp {
    Ptr<Allocator> default_allocator = &malloc_allocator;
}



#include <cstdio>

void print(Ref<const Bezier<V2f, 1>> bezier) {
    printf(
        "segment((%f, %f), (%f, %f)), ",
        bezier[0][0], bezier[0][1],
        bezier[1][0], bezier[1][1]
    );
}

void print(Ref<const Bezier<V2f, 2>> bezier) {
    printf(
        "curve( "
        "  %f*(1-t)^2 "
        "+ 2*%f*(1-t)*t "
        "+ %f*t^2, "
        "  %f*(1-t)^2 "
        "+ 2*%f*(1-t)*t "
        "+ %f*t^2, "
        "t, 0, 1), ",
        bezier[0][0], bezier[1][0], bezier[2][0],
        bezier[0][1], bezier[1][1], bezier[2][1]
    );
}

void print(Ref<const Bezier<V2f, 3>> bezier) {
    printf(
        "curve( "
        "  %f*(1-t)^3 "
        "+ 3*%f*(1-t)^2*t "
        "+ 3*%f*(1-t)*t^2 "
        "+ %f*t^3, "
        "  %f*(1-t)^3 "
        "+ 3*%f*(1-t)^2*t "
        "+ 3*%f*(1-t)*t^2 "
        "+ %f*t^3, "
        "t, 0, 1), ",
        bezier[0][0], bezier[1][0], bezier[2][0], bezier[3][0],
        bezier[0][1], bezier[1][1], bezier[2][1], bezier[3][1]
    );
}


void print(Ref<const Segment<V2f>> segment) {
    print(Bezier<V2f, 1>(segment));
}


template <typename Scalar, typename T>
Void split(
    Ref<const Bezier<T, 2>> bezier, Scalar t,
    Opt_Ptr<Bezier<T, 2>> s0,
    Opt_Ptr<Bezier<T, 2>> s1
) {
    auto l00 = bezier[0];
    auto l01 = bezier[1];
    auto l02 = bezier[2];
    auto l10 = lerp(l00, l01, t);
    auto l11 = lerp(l01, l02, t);
    auto l20 = lerp(l10, l11, t);
    if(s0.is_some()) {
        *s0.value = Bezier<T, 2>({ l00, l10, l20 });
    }
    if(s1.is_some()) {
        *s1.value = Bezier<T, 2>({ l20, l11, l02 });
    }
}


template <typename Scalar, typename T>
Void split(
    Ref<const Bezier<T, 3>> bezier, Scalar t,
    Opt_Ptr<Bezier<T, 3>> s0,
    Opt_Ptr<Bezier<T, 3>> s1
) {
    auto l00 = bezier[0];
    auto l01 = bezier[1];
    auto l02 = bezier[2];
    auto l03 = bezier[3];
    auto l10 = lerp(l00, l01, t);
    auto l11 = lerp(l01, l02, t);
    auto l12 = lerp(l02, l03, t);
    auto l20 = lerp(l10, l11, t);
    auto l21 = lerp(l11, l12, t);
    auto l30 = lerp(l20, l21, t);
    if(s0.is_some()) {
        *s0.value = Bezier<T, 3>({ l00, l10, l20, l30 });
    }
    if(s1.is_some()) {
        *s1.value = Bezier<T, 3>({ l30, l21, l12, l03 });
    }
}


Bool is_flat_enough(Ref<const Bezier<V2f, 2>> bezier, F32 tolerance) {
    auto err = squared(2.0f*bezier[1] - bezier[0] - bezier[2]);
    return err.x() + err.y() <= tolerance;
}

F32 make_flatten_b2_tolerance(F32 precision) {
    return 16.0f * squared(precision);
}


Bool is_flat_enough(Ref<const Bezier<V2f, 3>> bezier, F32 tolerance) {
    auto u = squared(3.0f*bezier[1] - 2.0f*bezier[0] - bezier[3]);
    auto v = squared(3.0f*bezier[2] - 2.0f*bezier[3] - bezier[0]);
    auto err = V2f(max(u, v));
    return err.x() + err.y() <= tolerance;
}

F32 make_flatten_b3_tolerance(F32 precision) {
    return 16.0f * squared(precision);
}


template <Usize degree>
Void flatten(Ref<const Bezier<V2f, degree>> bezier, F32 tolerance, Ref<List<Segment<V2f>>> segments) {
    if(is_flat_enough(bezier, tolerance)) {
        segments.append_new(Segment<V2f>({ bezier[0], bezier[degree] }));
    }
    else {
        Bezier<V2f, degree> l, r;
        split<F32, V2f>(bezier, 0.5f, &l, &r);
        flatten(l, tolerance, segments);
        flatten(r, tolerance, segments);
    }
}



#if 0
enum class Curve_Kind {
    bezier_1 = 1,
    bezier_2,
    bezier_3,
};

struct Baked_Path {
    struct Curve_Index {
        static constexpr Usize kind_bit_count = 2;

        Curve_Index(Curve_Kind kind, U32 index)
            : value(U32(kind) | (index << kind_bit_count)) {}

        Curve_Kind kind() const {
            return Curve_Kind(value & mask_ending_at<U32>(kind_bit_count));
        }

        U32 index() const {
            return value >> kind_bit_count;
        }

        U32 value;
    };

    struct Sub_Path {
        Range<U32> indices;
        Bool is_closed;
    };

    List<Bezier<V2f, 1>> b1s;
    List<Bezier<V2f, 2>> b2s;
    List<Bezier<V2f, 3>> b3s;
    List<Curve_Index> indices;
    List<Sub_Path> sub_paths;

    Path() {}
    LPP_MOVE_IS_DESTROY_CTORS(Path, Path);
};


struct Path {
};


//constexpr auto zero_tolerance = 8.0f*FLT_EPSILON;

Path compute_stroke(
    Ref<const Path> path,
    F32 left_offset, F32 right_offset,
    F32 tolerance
) {

    LPP_UNUSED(path, left_offset, right_offset, tolerance);

    /*
        - turn each sub-path into a new sub-path.
        - we should really output connected segments though. then you can stroke
          a stroke. also: Path (with sub-paths and indices) is expected to have
          consecutive segments. this proc also assumes that. so if we don't want
          to bother with that, we shouldn't return Path.
        - so: left outline, cap, right outline (in reverse), cap.
    */

    // degree reduction.
    auto quads = List<Bezier<V2f, 2>>();
    for(const auto& cubic : path.b3s) {
        LPP_UNUSED(cubic);
        //reduce_degree(cubic, tolerance, quads);
    }

    return {};

    #if 0

    // offsetting.
    auto left_outline  = List<Generic_Bezier>();
    auto right_outline = List<Generic_Bezier>();
    for(auto i : range_of(outline)) {
        const auto& curve = outline[i];

        if(curve.degree == 1) {
            auto bezier = get_bezier_1(curve);
            // TODO: zero precision?
            if(length_squared(bezier[1] - bezier[0]) == 0.0f) {
                left_outline.push_back(curve);
                right_outline.push_back(curve);
            }
            else {
                auto n = rotate_ccw(normalized(bezier[1] - bezier[0]));
                left_outline. push_back(generic_line(bezier[0] + left_offset*n,  bezier[1] + left_offset*n));
                right_outline.push_back(generic_line(bezier[0] - right_offset*n, bezier[1] - right_offset*n));
            }
        }
        else if(curve.degree == 2) {
            auto bezier = get_bezier_2(curve);
            auto l = offset(bezier, +left_offset, zero_tolerance);
            auto r = offset(bezier, -right_offset, zero_tolerance);
            left_outline. push_back(generic_quadratic(l[0], l[1], l[2]));
            right_outline.push_back(generic_quadratic(r[0], r[1], r[2]));
        }
        else {
            assert(false);
        }
    }

    // compute stroke by connecting end points.
    auto stroke = List<Generic_Bezier>();
    for(auto i : range_of(left_outline)) {
        const auto& current = left_outline[i];

        if(i > 0) {
            const auto& prev = left_outline[i - 1];
            stroke.push_back(generic_line(prev[prev.degree], current[0]));
        }

        stroke.push_back(current);
    }
    for(auto i : range_of(right_outline)) {
        auto current = right_outline[i];

        if(i > 0) {
            const auto& prev = right_outline[i - 1];
            stroke.push_back(generic_line(current[0], prev[prev.degree]));
        }

        std::reverse(&current.values[0], &current.values[current.degree + 1]);
        stroke.push_back(current);
    }

    // Bevel cap.
    if(is_closed && outline.size() > 1) {
        const auto& first_left = first(left_outline);
        const auto& last_left  = last(left_outline);
        stroke.push_back(generic_line(last_left[last_left.degree], first_left[0]));

        const auto& first_right = first(right_outline);
        const auto& last_right  = last(right_outline);
        stroke.push_back(generic_line(first_right[0], last_right[last_right.degree]));
    }
    // Bevel join.
    else {
        const auto& first_left  = first(left_outline);
        const auto& first_right = first(right_outline);
        stroke.push_back(generic_line(first_right[0], first_left[0]));

        const auto& last_left  = last(left_outline);
        const auto& last_right = last(right_outline);
        stroke.push_back(generic_line(last_left[last_left.degree], last_right[last_right.degree]));
    }

    return stroke;
    #endif
}
#endif


#include "rasterizer.hpp"
#include "msaa.hpp"


#include <chrono>


int main() {

    constexpr auto precision = 0.141f; // sqrt(1/(16*pi))

    auto my_b1s = List<Bezier<V2f, 1>>();
    auto my_b3s = List<Bezier<V2f, 3>>();
    my_b3s.append_new(Bezier<V2f, 3>({ V2f({125, 325}), V2f({150, 425}), V2f({300, 400}), V2f({300, 300}) }));
    my_b3s.append_new(Bezier<V2f, 3>({V2f({300, 300}), V2f({300, 200}), V2f({150, 175}), V2f({125, 275})}));
    my_b3s.append_new(Bezier<V2f, 3>({V2f({125, 275}), V2f({125, 225}), V2f({150, 125}), V2f({225, 125})}));
    my_b1s.append_new(Bezier<V2f, 1>({V2f({225, 125}), V2f({475, 125})}));
    my_b3s.append_new(Bezier<V2f, 3>({V2f({475, 125}), V2f({400, 200}), V2f({450, 275}), V2f({375, 300})}));
    my_b3s.append_new(Bezier<V2f, 3>({V2f({375, 300}), V2f({450, 325}), V2f({475, 351.2f}), V2f({475, 400})}));
    my_b3s.append_new(Bezier<V2f, 3>({V2f({475, 400}), V2f({475, 450}), V2f({450, 475}), V2f({400, 475})}));
    my_b1s.append_new(Bezier<V2f, 1>({V2f({400, 475}), V2f({225, 475})}));
    my_b3s.append_new(Bezier<V2f, 3>({V2f({225, 475}), V2f({150, 475}), V2f({125, 375}), V2f({125, 325})}));

    auto segments = List<Segment<V2f>>();
    segments.reserve(2048);

    #if 1
    flatten(my_b3s[0], make_flatten_b3_tolerance(precision), segments);
    flatten(my_b3s[1], make_flatten_b3_tolerance(precision), segments);
    flatten(my_b3s[2], make_flatten_b3_tolerance(precision), segments);
    segments.append_new(my_b1s[0]);
    flatten(my_b3s[3], make_flatten_b3_tolerance(precision), segments);
    flatten(my_b3s[4], make_flatten_b3_tolerance(precision), segments);
    flatten(my_b3s[5], make_flatten_b3_tolerance(precision), segments);
    segments.append_new(my_b1s[1]);
    flatten(my_b3s[6], make_flatten_b3_tolerance(precision), segments);
    #else
    for(const auto& b1 : my_b1s) {
        segments.append_new(b1);
    }
    for(const auto& b3 : my_b3s) {
        flatten(b3, make_flatten_b3_tolerance(precision), segments);
    }
    #endif

    if(0)
    for(auto& segment : segments) {
        segment.p0() = segment.p0() / 2.0f;
        segment.p1() = segment.p1() / 2.0f;
        static bool foo = false;
        if(foo) {
            segment.p0().y() -= 2e-6f;
            segment.p1().y() += 6.f;
        }
        else {
            segment.p1().y() -= 2e-6f;
            segment.p0().y() += 6.f;
        }
        foo = !foo;
    }

    auto lut = msaa::Lut::create(msaa::Samples::x32);
    auto sample_runs = List<msaa::Sample_Run>();

    msaa::rasterize(segments, lut, sample_runs);

    if(0) { return 0; }

    if(1)
    {
        auto width = U32(300);
        auto height = U32(300);
        auto image_msaa = Image<Color_Rgba>::create(width, height, lut.sample_count);
        auto image = Image<Color_Bgra>::create(width, height, 1);

        //memset(image_msaa.samples, -1, Usize(width*height*4*image_msaa.sample_count));

        auto t0 = std::chrono::high_resolution_clock::now();
        auto iters = 10'000;
        for(auto i : Range<int>(iters)) { LPP_UNUSED(i);
            //sample_runs.length = 0;
            //msaa::rasterize(segments, lut, sample_runs);

            //msaa::fill_opaque(image_msaa, sample_runs, V4f({ 1.0f, 0.7f, 0.2f, 1.0f }));

            //msaa::resolve(image, image_msaa);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        auto us = F64((t1-t0).count()) / F64(iters) / 1000.0;
        printf("%.3f us\n", us);

        if(0) { return 0; }

        msaa::fill_opaque(image_msaa, sample_runs, V4f({ 1.0f, 0.7f, 0.2f, 1.0f }));
        msaa::resolve(image, image_msaa, true);

        stbi_write_png("out.png", int(width), int(height), 4, image.samples, int(width)*4);
    }

    if(1) return 0;

    auto t0 = std::chrono::high_resolution_clock::now();
    auto iters = 10'000;
    auto r = Rasterizer();
    for(auto i : Range<int>(iters)) { LPP_UNUSED(i);
        //sample_runs.length = 0;
        //msaa::rasterize(segments, lut, sample_runs);
        r.run(segments);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    auto us = F64((t1-t0).count()) / F64(iters) / 1000.0;
    printf("%.3f us\n", us);

    return 0;
}

