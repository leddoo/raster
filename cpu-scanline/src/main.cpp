#include <common/common.hpp>
#include <common/math.hpp>
#include <common/slice.hpp>

#include <exception>
using Exception = std::exception;

#pragma warning(disable: 4201) // anonymous structs.


template <typename T>
T sign(T v) {
    if     (v < T(0)) { return T(-1); }
    else if(v > T(0)) { return T(1);  }
    else              { return T(0);  }
}


template <typename T>
union V2 {
    struct { T x; T y; };
    T values[2];

    V2()         : x(0),  y(0)  {}
    V2(T xy)     : x(xy), y(xy) {}
    V2(T x, T y) : x(x),  y(y)  {}

    template <typename U>
    explicit V2(const V2<U>& other) : x(T(other.x)), y(T(other.y)) {}


    T& operator[](Uint index) {
        assert(index < 2);
        return this->values[index];
    }

    const T& operator[](Uint index) const {
        assert(index < 2);
        return this->values[index];
    }
};

using V2f = V2<Float32>;
using V2s = V2<Sint32>;
using V2u = V2<Uint32>;


template <typename T> V2<T> operator+(V2<T> a, V2<T> b) { return { a.x + b.x, a.y + b.y }; }
template <typename T> V2<T> operator-(V2<T> a, V2<T> b) { return { a.x - b.x, a.y - b.y }; }
template <typename T> V2<T> operator*(V2<T> a, V2<T> b) { return { a.x * b.x, a.y * b.y }; }
template <typename T> V2<T> operator/(V2<T> a, V2<T> b) { return { a.x / b.x, a.y / b.y }; }
template <typename T> V2<T> operator*(T a, V2<T> b) { return { a * b.x, a * b.y }; }
template <typename T> V2<T> operator*(V2<T> a, T b) { return { a.x * b, a.y * b }; }
template <typename T> V2<T> operator/(V2<T> a, T b) { return { a.x / b, a.y / b }; }

template <typename T> V2<T> floor(V2<T> a) { return { floor(a.x), floor(a.y) }; }
template <typename T> V2<T> ceil(V2<T> a)  { return { ceil(a.x),  ceil(a.y)  }; }
template <typename T> V2<T> abs(V2<T> a)   { return { abs(a.x),   abs(a.y)   }; }
template <typename T> V2<T> sign(V2<T> a)  { return { sign(a.x),  sign(a.y)  }; }
template <typename T> V2<T> min(V2<T> a, V2<T> b) { return { min(a.x, b.x), min(a.y, b.y) }; }
template <typename T> V2<T> max(V2<T> a, V2<T> b) { return { max(a.x, b.x), max(a.y, b.y) }; }


struct Generic_Bezier {
    static constexpr Uint max_point_count = 4;

    Uint32                      degree;
    Array<V2f, max_point_count> values;
};

Generic_Bezier generic_line(V2f p0, V2f p1) {
    auto result = Generic_Bezier();
    result.degree = 1;
    result.values[0] = p0;
    result.values[1] = p1;
    return result;
}

Generic_Bezier generic_quadratic(V2f p0, V2f p1, V2f p2) {
    auto result = Generic_Bezier();
    result.degree = 2;
    result.values[0] = p0;
    result.values[1] = p1;
    result.values[2] = p2;
    return result;
}

Generic_Bezier generic_cubic(V2f p0, V2f p1, V2f p2, V2f p3) {
    auto result = Generic_Bezier();
    result.degree = 3;
    result.values[0] = p0;
    result.values[1] = p1;
    result.values[2] = p2;
    result.values[3] = p3;
    return result;
}



Float32 bernstein_1_0(Float32 t) { auto x = 1.f - t; return x; }
Float32 bernstein_1_1(Float32 t) {                   return t; }

Float32 bernstein_2_0(Float32 t) { auto x = 1.f - t; return x*x;   }
Float32 bernstein_2_1(Float32 t) { auto x = 1.f - t; return 2*x*t; }
Float32 bernstein_2_2(Float32 t) {                   return t*t;   }

Float32 bernstein_3_0(Float32 t) { auto x = 1.f - t; return x*x*x;   }
Float32 bernstein_3_1(Float32 t) { auto x = 1.f - t; return 3*x*x*t; }
Float32 bernstein_3_2(Float32 t) { auto x = 1.f - t; return 3*x*t*t; }
Float32 bernstein_3_3(Float32 t) {                    return t*t*t;   }


template <typename T>
T evaluate_bezier_1(const T values[2], Float32 t) {
    auto result =
          bernstein_1_0(t)*values[0]
        + bernstein_1_1(t)*values[1];
    return result;
}

template <typename T>
T evaluate_bezier_2(const T values[3], Float32 t) {
    auto result =
          bernstein_2_0(t)*values[0]
        + bernstein_2_1(t)*values[1]
        + bernstein_2_2(t)*values[2];
    return result;
}

template <typename T>
T evaluate_bezier_3(const T values[4], Float32 t) {
    auto result =
          bernstein_3_0(t)*values[0]
        + bernstein_3_1(t)*values[1]
        + bernstein_3_2(t)*values[2]
        + bernstein_3_3(t)*values[3];
    return result;
}


V2f evaluate(const Generic_Bezier& curve, Float32 t) {
    switch (curve.degree) {
        case 1: return evaluate_bezier_1(curve.values.data(), t);
        case 2: return evaluate_bezier_2(curve.values.data(), t);
        case 3: return evaluate_bezier_3(curve.values.data(), t);
        default: throw Exception();
    }
}


void get_poly_coefficients_1(
    const Generic_Bezier& curve, Uint axis,
    Float32& a1, Float32& a0
) {
    auto v0 = curve.values[0][axis];
    auto v1 = curve.values[1][axis];
    a1 = v1 - v0;
    a0 = v0;
}

void get_poly_coefficients_2(
    const Generic_Bezier& curve, Uint axis,
    Float32& a2, Float32& a1, Float32& a0
) {
    auto v0 = curve.values[0][axis];
    auto v1 = curve.values[1][axis];
    auto v2 = curve.values[2][axis];
    a2 = v0 - 2.f*v1 + v2;
    a1 = 2.f*v1 - 2.f*v0;
    a0 = v0;
}

void get_poly_coefficients_3(
    const Generic_Bezier& curve, Uint axis,
    Float32& a3, Float32& a2, Float32& a1, Float32& a0
) {
    auto v0 = curve.values[0][axis];
    auto v1 = curve.values[1][axis];
    auto v2 = curve.values[2][axis];
    auto v3 = curve.values[3][axis];
    a3 = -v0 + 3.f*v1 - 3.f*v2 + v3;
    a2 = 3.f*v0 - 6.f*v1 + 3.f*v2;
    a1 = 3.f*v1 - 3.f*v0;
    a0 = v0;
}

void get_poly_coefficients(
    const Generic_Bezier& curve, Uint axis,
    Float32& a3, Float32& a2, Float32& a1, Float32& a0
) {
    switch (curve.degree) {
        case 1: get_poly_coefficients_1(curve, axis, a1, a0); break;
        case 2: get_poly_coefficients_2(curve, axis, a2, a1, a0); break;
        case 3: get_poly_coefficients_3(curve, axis, a3, a2, a1, a0); break;
        default: throw Exception();
    }
}


Float32 evaluate_poly_1(const Float32 values[2], Float32 t) {
    return values[1]*t + values[0];
}

Float32 evaluate_poly_2(const Float32 values[3], Float32 t) {
    return (values[2]*t + values[1])*t + values[0];
}

Float32 evaluate_poly_3(const Float32 values[4], Float32 t) {
    return ((values[3]*t + values[2])*t + values[1])*t + values[0];
}



constexpr Float32 tolerance = 1e-6f;


Uint find_roots_poly_1(
    Float32 a1, Float32 a0,
    Float32& r0
) {
    if(abs(a1) < tolerance) {
        return 0;
    }
    else {
        r0 = -a0/a1;
        return 1;
    }
}

Uint find_roots_poly_2(
    Float32 a2, Float32 a1, Float32 a0,
    Float32& r0, Float32& r1
) {
    if(abs(a2) < tolerance) {
        return find_roots_poly_1(a1, a0, r0);
    }

    auto p = a1/a2;
    auto q = a0/a2;

    auto x0 = -p/2.f;
    auto discriminant = squared(p/2.f) - q;
    if(discriminant < squared(tolerance)) {
        r0 = x0;
        return 1;
    }
    else {
        auto delta = sqrtf(discriminant);
        r0 = x0 - delta;
        r1 = x0 + delta;
        return 2;
    }
}


Uint find_derivative_roots_bezier_2(Float32 v0, Float32 v1, Float32 v2, Float32& r0) {
    return find_roots_poly_1(
        (v0 - v1) + (v2 - v1),
        (v1 - v0),
        r0
    );
}

Uint find_derivative_roots_bezier_2(const V2f values[3], Uint32 axis, Float32& r0) {
    return find_derivative_roots_bezier_2(values[0][axis], values[1][axis], values[2][axis], r0);
}


Uint find_derivative_roots_bezier_3(Float32 v0, Float32 v1, Float32 v2, Float32 v3, Float32& r0, Float32& r1) {
    return find_roots_poly_2(
        3.f*(v1 - v2) + (v3 - v0),
        2.f*((v0 - v1) + (v2 - v1)),
        v1 - v0,
        r0, r1
    );
}

Uint find_derivative_roots_bezier_3(const V2f values[4], Uint32 axis, Float32& r0, Float32& r1) {
    return find_derivative_roots_bezier_3(values[0][axis], values[1][axis], values[2][axis], values[3][axis], r0, r1);
}


Uint find_derivative_roots(const Generic_Bezier& curve, Uint32 axis, Float32& t0, Float32& t1) {
    auto root_count = Uint();
    switch(curve.degree) {
        case 1: root_count = 0; break; // derivative is constant.
        case 2: root_count = find_derivative_roots_bezier_2(curve.values.data(), axis, t0); break;
        case 3: root_count = find_derivative_roots_bezier_3(curve.values.data(), axis, t0, t1); break;
        default: throw Exception();
    }
    return root_count;
}

template <typename T, typename F>
void insertion_sort(Slice<T> slice, F leq) {
    for(auto i : Range<Uint>(1, slice.count)) {
        auto j = i;
        while(j > 0 && !leq(slice.values[j - 1], slice.values[j])) {
            swap(slice.values[j - 1], slice.values[j]);
            j += 1;
        }
    }
}


int main() {

    auto path = List<Generic_Bezier> {
        generic_line({10.f, 10.f}, {20.f, 10.f}),
        generic_quadratic({20.f, 10.f}, {23.5f, 15.f}, {30.f, 10.f}),
        generic_line({30.f, 10.f}, {37.5f, 15.f}),
        generic_cubic({37.5f, 15.f}, {28.f, 30.f}, {10.f, 22.f}, {10.f, 10.f}),
    };



    // Find monotone segments. (Ranges where the derivative's sign is constant -> between the roots.)

    constexpr Uint max_cuts_per_axis = 2;
    constexpr Uint max_cut_count = 2*max_cuts_per_axis;
    struct Cut {
        Float32 t;
        Uint32  axis;
    };

    auto curve_cuts = List<Array<Cut, max_cut_count>>(path.size());
    for(auto curve_index : range_of(path)) {
        auto& cuts = curve_cuts[curve_index];

        auto total_cuts = Uint(0);

        for(auto axis : Range<Uint32>(2)) {
            auto cuts_begin = axis*max_cuts_per_axis;

            auto& cut_0 = cuts[cuts_begin + 0];
            auto& cut_1 = cuts[cuts_begin + 1];
            // Initialize cuts to make sorting work.
            cut_0.t    = cut_1.t    = 1.f;
            cut_0.axis = cut_1.axis = axis;

            total_cuts += find_derivative_roots(path[curve_index], axis, cut_0.t, cut_1.t);
        }

        // Sort cuts by t. (We have to sort the entire array. Eg: [1, 1, 0.5, 0.75])
        insertion_sort(make_slice(cuts), [](const auto& a, const auto& b) { return a.t <= b.t; });
    }



    // Find boundary fragments by rasterizing the curves.

    struct Boundary_Fragment {
        V2s     position;
        Float32 t0, t1;
        Uint32  curve_index;
    };

    auto fragments = List<Boundary_Fragment>();
    for(auto curve_index : range_of(path)) {
        auto& curve = path[curve_index];
        auto& cuts  = curve_cuts[curve_index];

        // Note: cuts are sorted by t.

        // Skip to first positive cut. (This may skip all cuts.)
        auto cut_cursor = Uint(0);
        while(cut_cursor < max_cut_count && cuts[cut_cursor].t <= 0.f + tolerance) {
            cut_cursor += 1;
        }

        printf("curve.\n");

        // Process segments.
        auto cut_t0 = 0.f;
        while(cut_t0 < 1.f) {

            auto cut_t1 = 1.f;
            if(cut_cursor < max_cut_count && cuts[cut_cursor].t < 1.f - tolerance) {
                cut_t1 = cuts[cut_cursor].t;
                cut_cursor += 1;
            }

            printf("segment: %f %f.\n", cut_t0, cut_t1);

            // Rasterize segment.

            const auto p0 = evaluate(curve, cut_t0);
            const auto p1 = evaluate(curve, cut_t1);

            const auto step = sign(p1 - p0);

            // these are inclusive, hence the first/last nomenclature.
            const auto first_fragment = V2s(floor(p0));
            const auto last_fragment  = V2s(floor(p1));

            // fence post:
            // (end - begin) is post_count. step_count is post_count - 1.
            // => (last - first) is step_count.
            const auto step_count = abs(last_fragment - first_fragment);

            // each step adds a fragment plus the starting fragment.
            const auto frag_count = step_count.x + step_count.y + 1;

            printf("p0: %f %f\n", p0.x, p0.y);
            printf("p1: %f %f\n", p1.x, p1.y);

            printf("step:  %f %f\n", step.x, step.y);
            printf("first: %d %d\n", first_fragment.x, first_fragment.y);
            printf("last:  %d %d\n", last_fragment.x, last_fragment.y);
            printf("count: %d\n", frag_count);


            auto fragment_cursor = first_fragment;
            auto steps_remaining = step_count;

            // the fragment_cursor is always in the bottom right of the fragment.
            // `fragment_cursor + grid_offset + step` yields the next grid lines.
            auto grid_offset = V2f(0.5f) - 0.5f*step;


            auto find_next_t = [&](Uint axis) -> Float32 {
                auto next_pos = fragment_cursor[axis] + grid_offset[axis] + step[axis];

                auto t_min = 2.f;

                // segment is monotonic -> at most one root in segment interval.
                // root finding may however return roots outside of the segment.

                auto clamp_t = [=](Float32 t) {
                    if     (t < cut_t0 - tolerance) { return 2.f; }
                    else if(t > cut_t1 + tolerance) { return 2.f; }
                    return clamp(t, cut_t0, cut_t1);
                };

                switch (curve.degree) {
                    case 1: {
                        Float32 poly[2] = {};
                        get_poly_coefficients_1(curve, axis, poly[1], poly[0]);
                        poly[0] -= next_pos;

                        auto r0 = 2.f;
                        find_roots_poly_1(poly[1], poly[0], r0);
                        t_min = clamp_t(r0);
                    } break;

                    case 2: {
                        Float32 poly[3] = {};
                        get_poly_coefficients_2(curve, axis, poly[2], poly[1], poly[0]);
                        poly[0] -= next_pos;

                        auto r0 = 2.f, r1 = 2.f;
                        find_roots_poly_2(poly[2], poly[1], poly[0], r0, r1);
                        t_min = min(clamp_t(r0), clamp_t(r1));
                    } break;

                    case 3: {
                        // Newton raphson.
                        constexpr Uint iter_count = 8;

                        Float32 poly[4] = {};
                        get_poly_coefficients_3(curve, axis, poly[3], poly[2], poly[1], poly[0]);
                        poly[0] -= next_pos;

                        Float32 derivative[3] = {};
                        derivative[2] = 3.f*poly[3];
                        derivative[1] = 2.f*poly[2];
                        derivative[0] = 1.f*poly[1];

                        auto t = 0.5f*(cut_t0 + cut_t1);
                        for(auto i : Range<Uint>(iter_count)) {
                            UNUSED(i);
                            t -= evaluate_poly_3(poly, t)/evaluate_poly_2(derivative, t);
                        }

                        t_min = clamp_t(t);
                    } break;

                    default: throw Exception();
                }

                if(t_min > 1.f) {
                    if(t_min <= 1.f + tolerance) {
                        t_min = 1.f;
                    }
                    else {
                        t_min = 2.f;
                    }
                }

                return t_min;
            };

            auto next_t = V2f(2.f);
            for(auto axis : Range<Uint>(2)) {
                next_t[axis] = find_next_t(axis);
            }


            while(true) {
                // emit fragment.
                //printf("fragment: %d %d\n", fragment_cursor.x, fragment_cursor.y);
                {
                    auto x = fragment_cursor.x;
                    auto y = fragment_cursor.y;
                    printf("Polygon((%d, %d), (%d, %d), (%d, %d), (%d, %d)), ",
                                     x,  y,    x+1,y,    x+1,y+1,  x,  y+1);
                }

                if(steps_remaining.x <= 0 && steps_remaining.y <= 0) {
                    break;
                }


                auto min_axis = Uint(next_t[0] < next_t[1] ? 0 : 1);

                // take a step.
                assert(steps_remaining[min_axis] > 0);
                fragment_cursor[min_axis] += Sint32(step[min_axis]);
                steps_remaining[min_axis] -= 1;

                // advance time, so find_next_t does not give us previous results.
                cut_t0 = next_t[min_axis];

                next_t[min_axis] = find_next_t(min_axis);
            }

            cut_t0 = cut_t1;

            printf("\n");
        }

        printf("\n");
    }


    /*
        winding number computation:
            - assume boundary fragments are sorted by y, then x.
            - walk boundary fragments.
            - whenever y changes, start a new line.
            - on each line:
                - start with current winding = 0.
                - for each fragment:
                    - compute winding in the center (one sample).
                    - compute winding at right edge -> current winding -> winding at left edge of next fragment.
                    - emit pixel for fragment.
                    - emit span if filled and next fragment is not immidately following.
    */

    return 0;
}
