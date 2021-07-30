#include <common/common.hpp>
#include <common/math.hpp>
#include <common/slice.hpp>

#include <exception>
using Exception = std::exception;

#include "lina.hpp"
#include "poly.hpp"
#include "bezier.hpp"



Bezier<Float32, 0> get_component_bezier(const Bezier<V2f, 0>& bezier, Uint axis) {
    return Bezier<Float32, 0>{ bezier[0][axis] };
}

Bezier<Float32, 1> get_component_bezier(const Bezier<V2f, 1>& bezier, Uint axis) {
    return Bezier<Float32, 1>{ bezier[0][axis], bezier[1][axis] };
}

Bezier<Float32, 2> get_component_bezier(const Bezier<V2f, 2>& bezier, Uint axis) {
    return Bezier<Float32, 2>{ bezier[0][axis], bezier[1][axis], bezier[2][axis] };
}

Bezier<Float32, 3> get_component_bezier(const Bezier<V2f, 3>& bezier, Uint axis) {
    return Bezier<Float32, 3>{ bezier[0][axis], bezier[1][axis], bezier[2][axis], bezier[3][axis] };
}



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


Bezier<V2f, 0> get_bezier_0(const Generic_Bezier& curve) {
    return Bezier<V2f, 0>{ curve.values[0] };
}

Bezier<V2f, 1> get_bezier_1(const Generic_Bezier& curve) {
    return Bezier<V2f, 1>{ curve.values[0], curve.values[1] };
}

Bezier<V2f, 2> get_bezier_2(const Generic_Bezier& curve) {
    return Bezier<V2f, 2>{ curve.values[0], curve.values[1], curve.values[2] };
}

Bezier<V2f, 3> get_bezier_3(const Generic_Bezier& curve) {
    return Bezier<V2f, 3>{ curve.values[0], curve.values[1], curve.values[2], curve.values[3] };
}


V2f evaluate(const Generic_Bezier& curve, Float32 t) {
    switch (curve.degree) {
        case 0: { return evaluate(get_bezier_0(curve), t); } break;
        case 1: { return evaluate(get_bezier_1(curve), t); } break;
        case 2: { return evaluate(get_bezier_2(curve), t); } break;
        case 3: { return evaluate(get_bezier_3(curve), t); } break;

        default: throw Exception();
    }
}


constexpr Float32 tolerance = 1e-6f;


Uint find_derivative_roots(const Generic_Bezier& curve, Uint32 axis, Float32& t0, Float32& t1) {
    auto root_count = Uint();
    switch(curve.degree) {
        case 0: case 1: {
            root_count = 0;
        } break;

        case 2: {
            auto bezier = get_bezier_2(curve);
            auto component_bezier = get_component_bezier(bezier, axis);
            auto derivative = derive<Float32>(component_bezier);
            auto poly = get_poly<Float32>(derivative);
            root_count = find_roots(poly, t0, tolerance);
        } break;

        case 3: {
            auto bezier = get_bezier_3(curve);
            auto component_bezier = get_component_bezier(bezier, axis);
            auto derivative = derive<Float32>(component_bezier);
            auto poly = get_poly<Float32>(derivative);
            root_count = find_roots(poly, t0, t1, tolerance);
        } break;
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


Bool find_segments_intersection(V2f a0, V2f a1, V2f b0, V2f b1, V2f* ts) {
    auto matrix = M2f::from_columns(a1 - a0, b0 - b1);
    auto inverse = M2f();
    if(invert_matrix2(matrix, &inverse, tolerance) == false) {
        return false;
    }

    auto _ts = inverse*(b0 - a0);
    if(in_interval(_ts.x, 0, 1, tolerance) && in_interval(_ts.y, 0, 1, tolerance)) {
        if(ts != nullptr) {
            *ts = _ts;
        }
        return true;
    }
    else {
        return false;
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



    struct Boundary_Fragment {
        V2s     position;
        Float32 t0;
        Uint32  curve_index;

        Sint32 in_winding;   // winding at (0, 0.5).
        Bool   winding_sign; // curve goes up?
        Bool   out_mask;     // ((0, 0.5), (1, 0.5)) hits curve?
        Bool   sample_mask;  // ((0, 0.5), (0.5, 0.5)) hits curve?

        Sint32 x() const { return position.x; }
        Sint32 y() const { return position.y; }
    };

    // Find boundary fragments by rasterizing the curves.
    // Set position, t0, curve_index. (zero init rest)

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

        // Process segments.
        auto cut_t0 = 0.f;
        while(cut_t0 < 1.f) {

            auto cut_t1 = 1.f;
            if(cut_cursor < max_cut_count && cuts[cut_cursor].t < 1.f - tolerance) {
                cut_t1 = cuts[cut_cursor].t;
                cut_cursor += 1;
            }

            // Rasterize segment.

            const auto p0 = evaluate(curve, cut_t0);
            const auto p1 = evaluate(curve, cut_t1);

            // these are inclusive, hence the first/last nomenclature.
            const auto first_fragment = V2s(floor(p0));
            const auto last_fragment  = V2s(floor(p1));

            const auto step = sign(p1 - p0);
            const auto step_count = abs(last_fragment - first_fragment);

            // each step adds a fragment plus the starting fragment.
            const auto frag_count = step_count.x + step_count.y + 1;


            auto steps_remaining = step_count;
            auto fragment_cursor = first_fragment;

            const auto find_next_t = [&](Uint axis) -> Float32 {
                // the fragment_cursor is always in the bottom right of the fragment.
                // `fragment_cursor + grid_offset + step` yields the next grid lines.
                const auto grid_offset = V2f(0.5f) - 0.5f*step;

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
                        auto bezier = get_bezier_1(curve);
                        auto component_bezier = get_component_bezier(bezier, axis);
                        auto poly = get_poly<Float32>(component_bezier);
                        poly[0] -= next_pos;

                        auto r0 = 2.f;
                        find_roots(poly, r0, tolerance);
                        t_min = clamp_t(r0);
                    } break;

                    case 2: {
                        auto bezier = get_bezier_2(curve);
                        auto component_bezier = get_component_bezier(bezier, axis);
                        auto poly = get_poly<Float32>(component_bezier);
                        poly[0] -= next_pos;

                        auto r0 = 2.f, r1 = 2.f;
                        find_roots(poly, r0, r1, tolerance);
                        t_min = min(clamp_t(r0), clamp_t(r1));
                    } break;

                    case 3: {
                        constexpr Uint iter_count = 8;

                        auto bezier = get_bezier_3(curve);
                        auto component_bezier = get_component_bezier(bezier, axis);
                        auto poly = get_poly<Float32>(component_bezier);
                        poly[0] -= next_pos;

                        auto derivative = derive<Float32>(poly);

                        // Newton raphson.
                        auto t = 0.5f*(cut_t0 + cut_t1);
                        for(auto i : Range<Uint>(iter_count)) {
                            UNUSED(i);
                            t -= evaluate(poly, t)/evaluate(derivative, t);
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

            auto next_t = V2f(find_next_t(0), find_next_t(1));

            for(auto i : Range<Uint>(frag_count)) {
                UNUSED(i);

                auto min_axis = Uint(next_t[0] < next_t[1] ? 0 : 1);
                auto step_t   = next_t[min_axis];

                auto fragment = Boundary_Fragment();
                fragment.position    = fragment_cursor;
                fragment.t0          = cut_t0;
                fragment.curve_index = (Uint32)curve_index;
                fragments.push_back(fragment);

                if(steps_remaining[min_axis] > 0) {
                    // take a step.
                    fragment_cursor[min_axis] += Sint32(step[min_axis]);
                    steps_remaining[min_axis] -= 1;

                    // advance time, so find_next_t does not give us previous results.
                    cut_t0 = step_t;

                    next_t[min_axis] = find_next_t(min_axis);
                }
                else {
                    next_t[min_axis] = 2.f;
                }
            }

            cut_t0 = cut_t1;
        }
    }


    #if 0
        for(auto& fragment : fragments) {
            auto x = fragment.position.x;
            auto y = fragment.position.y;
            printf("Polygon((%d, %d),(%d, %d),(%d, %d),(%d, %d)), ",
                x, y, x+1, y, x+1, y+1, x, y+1
            );
        }
        printf("\n");

        for(auto i : range_of(fragments)) {
            auto t0 = fragments[i].t0;
            auto t1 = 1.f;
            if(i + 1 < fragments.size() && fragments[i].curve_index == fragments[i + 1].curve_index) {
                t1 = fragments[i + 1].t0;
            }
            auto p0 = evaluate(path[fragments[i].curve_index], t0);
            auto p1 = evaluate(path[fragments[i].curve_index], t1);
            printf("Segment((%f, %f), (%f, %f)), ",
                p0.x, p0.y, p1.x, p1.y
            );
        }
        printf("\n");

        for(auto i : range_of(fragments)) {
            auto t0 = fragments[i].t0;
            auto t1 = 1.f;
            if(i + 1 < fragments.size() && fragments[i].curve_index == fragments[i + 1].curve_index) {
                t1 = fragments[i + 1].t0;
            }
            auto p0 = evaluate(path[fragments[i].curve_index], t0);
            auto p1 = evaluate(path[fragments[i].curve_index], t1);
            printf("Point({%f, %f}), ", p0.x, p0.y);
        }
        printf("\n");

        for(auto i : range_of(fragments)) {
            auto t0 = fragments[i].t0;
            auto t1 = 1.f;
            if(i + 1 < fragments.size() && fragments[i].curve_index == fragments[i + 1].curve_index) {
                t1 = fragments[i + 1].t0;
            }
            auto p0 = evaluate(path[fragments[i].curve_index], t0);
            auto p1 = evaluate(path[fragments[i].curve_index], t1);
            printf("Point({%f, %f}), ", p1.x, p1.y);
        }
        printf("\n");
    #endif


    // compute per fragment winding changes and sample mask.
    for(auto i : range_of(fragments)) {
        auto& fragment = fragments[i];

        auto t0 = fragments[i].t0;
        auto t1 = 1.f;
        if(i + 1 < fragments.size() && fragment.curve_index == fragments[i + 1].curve_index) {
            t1 = fragments[i + 1].t0;
        }

        auto p0 = evaluate(path[fragments[i].curve_index], t0);
        auto p1 = evaluate(path[fragments[i].curve_index], t1);
        auto c0 = p0 - V2f(fragment.position);
        auto c1 = p1 - V2f(fragment.position);

        fragment.winding_sign = (p1.y - p0.y) < 0.f;
        fragment.out_mask     = find_segments_intersection(c0, c1, V2f(0.f, 0.5f), V2f(1.f, 0.5f), nullptr);
        fragment.sample_mask  = find_segments_intersection(c0, c1, V2f(0.f, 0.5f), V2f(0.5f, 0.5f), nullptr);
    }


    // sort boundary fragments by y then x.
    {
        struct Compare_Boundary_Fragments {
            Bool operator()(const Boundary_Fragment& a, const Boundary_Fragment& b) {
                if(a.y() != b.y()) { return a.y() < b.y(); }
                else               { return a.x() < b.x(); }
            }
        };
        std::sort(fragments.begin(), fragments.end(), Compare_Boundary_Fragments());
    }


    // scan pixel lines.
    {
        auto scan_winding = Sint32();
        auto scan_line    = Sint32();
        auto scan_x       = Sint32();

        auto i = Uint(0);
        while(i < fragments.size()) {
            auto position = fragments[i].position;

            if(position.y != scan_line) {
                assert(scan_winding == 0);
                scan_winding = 0;
                scan_line = fragments[i].y();
            }
            else if(position.x > scan_x + 1 && scan_winding != 0) {
                // Output solid span.
                auto x0 = scan_x;
                auto x1 = position.x; // exclusive.
                auto y  = scan_line;
                printf("Polygon((%d, %d),(%d, %d),(%d, %d),(%d, %d)), ",
                    x0, y, x1, y, x1, y+1, x0, y+1
                );
            }


            // Accumulate winding changes for this pixel.
            auto delta_out_winding    = Sint32(0);
            auto delta_sample_winding = Sint32(0);
            while(i < fragments.size() && fragments[i].x() == position.x) {
                auto sign = fragments[i].winding_sign ? -1 : 1;
                delta_out_winding    += sign*fragments[i].out_mask;
                delta_sample_winding += sign*fragments[i].sample_mask;
                i += 1;
            }

            auto sample_winding = scan_winding + delta_sample_winding;
            if(sample_winding != 0) {
                auto x = position.x;
                auto y = position.y;
                printf("Polygon((%d, %d),(%d, %d),(%d, %d),(%d, %d)), ",
                    x, y, x+1, y, x+1, y+1, x, y+1
                );
            }

            scan_winding    += delta_out_winding;
            scan_x           = position.x + 1;
        }
    }


    return 0;
}
