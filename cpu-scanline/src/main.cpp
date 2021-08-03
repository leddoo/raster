#define _CRT_SECURE_NO_WARNINGS
#include <common/common.hpp>
#include <common/math.hpp>
#include <common/slice.hpp>

#include <exception>
using Exception = std::exception;

#include "lina.hpp"
#include "poly.hpp"
#include "bezier.hpp"




struct Generic_Bezier {
    static constexpr Uint max_point_count = 4;

    Uint32                      degree;
    Array<V2f, max_point_count> values;

    V2f& operator[](Uint index) {
        assert(index <= this->degree);
        return this->values[index];
    }

    const V2f& operator[](Uint index) const {
        assert(index <= this->degree);
        return this->values[index];
    }
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
    assert(curve.degree == 0);
    return Bezier<V2f, 0>{ curve.values[0] };
}

Bezier<V2f, 1> get_bezier_1(const Generic_Bezier& curve) {
    assert(curve.degree == 1);
    return Bezier<V2f, 1>{ curve.values[0], curve.values[1] };
}

Bezier<V2f, 2> get_bezier_2(const Generic_Bezier& curve) {
    assert(curve.degree == 2);
    return Bezier<V2f, 2>{ curve.values[0], curve.values[1], curve.values[2] };
}

Bezier<V2f, 3> get_bezier_3(const Generic_Bezier& curve) {
    assert(curve.degree == 3);
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


Uint find_derivative_roots(const Generic_Bezier& curve, Uint32 axis, Float32& t0, Float32& t1, Float32 tolerance) {
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




void print(const Bezier<V2f, 1>& bezier) {
    printf(
        "segment((%f, %f), (%f, %f)), ",
        bezier[0][0], bezier[0][1],
        bezier[1][0], bezier[1][1]
    );
}

void print(const Bezier<V2f, 2>& bezier) {
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


template <typename T, typename F>
void insertion_sort(Slice<T> slice, F leq) {
    for(auto i : Range<Uint>(1, slice.count)) {
        auto j = i;
        while(j > 0 && !leq(slice[j - 1], slice[j])) {
            swap(slice[j - 1], slice[j]);
            j -= 1;
        }
    }
}


List<Generic_Bezier> compute_stroke(
    const List<Generic_Bezier>& path, Bool is_closed,
    Float32 left_offset, Float32 right_offset,
    Float32 tolerance
) {
    // degree reduction.
    auto outline = List<Generic_Bezier>();
    for(const auto& curve : path) {
        if(curve.degree == 3) {
            auto quads = reduce_degree(get_bezier_3(curve), 0.1f);
            for(const auto& q : quads) {
                outline.push_back(generic_quadratic(q[0], q[1], q[2]));
            }
        }
        else {
            outline.push_back(curve);
        }
    }

    // offsetting.
    auto left_outline  = List<Generic_Bezier>();
    auto right_outline = List<Generic_Bezier>();
    for(auto i : range_of(outline)) {
        const auto& curve = outline[i];

        if(curve.degree == 1) {
            auto bezier = get_bezier_1(curve);
            auto n = rotate_ccw(normalized(bezier[1] - bezier[0]));
            left_outline. push_back(generic_line(bezier[0] + left_offset*n,  bezier[1] + left_offset*n));
            right_outline.push_back(generic_line(bezier[0] - right_offset*n, bezier[1] - right_offset*n));
        }
        else if(curve.degree == 2) {
            auto bezier = get_bezier_2(curve);
            auto l = offset(bezier, +left_offset, tolerance);
            auto r = offset(bezier, -right_offset, tolerance);
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
}



constexpr Uint max_cuts_per_axis = 2;
constexpr Uint max_cut_count = 2*max_cuts_per_axis;
struct Cut {
    Float32 t;
    Uint32  axis;
};

// monotone: Ranges where the derivative's sign is constant -> between the roots.

Array<Cut, max_cut_count> find_monotone_segments(
    const Generic_Bezier& curve,
    Float32 tolerance
) {
    auto cuts = Array<Cut, max_cut_count>();

    for(auto axis : Range<Uint32>(2)) {
        auto cuts_begin = axis*max_cuts_per_axis;

        auto& cut_0 = cuts[cuts_begin + 0];
        auto& cut_1 = cuts[cuts_begin + 1];
        // Initialize cuts to make sorting work.
        cut_0.t    = cut_1.t    = 1.f;
        cut_0.axis = cut_1.axis = axis;

        find_derivative_roots(curve, axis, cut_0.t, cut_1.t, tolerance);
    }

    // Sort cuts by t. (We have to sort the entire array. Eg: [1, 1, 0.5, 0.75])
    insertion_sort(make_slice(cuts), [](const auto& a, const auto& b) { return a.t <= b.t; });

    return cuts;
}


List<Array<Cut, max_cut_count>> find_monotone_segments(
    const List<Generic_Bezier>& path,
    Float32 tolerance
) {
    auto curve_cuts = List<Array<Cut, max_cut_count>>(path.size());
    for(auto curve_index : range_of(path)) {
        curve_cuts[curve_index] = find_monotone_segments(path[curve_index], tolerance);
    }
    return curve_cuts;
}



struct Boundary_Fragment {
    V2s     position;
    Float32 t0;
    Uint32  curve_index;

    Sint32 winding_sign;
    Bool   out_mask;     // ((0, 0.5), (1, 0.5)) hits curve?
    Bool   sample_mask;  // ((0, 0.5), (0.5, 0.5)) hits curve?

    Sint32 x() const { return position.x; }
    Sint32 y() const { return position.y; }
};


List<Boundary_Fragment> find_boundary_fragments(
    const List<Generic_Bezier>& path,
    const List<Array<Cut, max_cut_count>>& curve_cuts,
    Float32 tolerance
) {
    auto fragments = List<Boundary_Fragment>();

    // Find boundary fragments by rasterizing the curves.
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

    return fragments;
}


void compute_winding(
    List<Boundary_Fragment>& fragments,
    const List<Generic_Bezier>& path,
    Float32 tolerance
) {
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

        auto normalized_0 = p0 - V2f(fragment.position);
        auto normalized_1 = p1 - V2f(fragment.position);

        // Sort coordinates to make hit testing curve direction independent.
        auto test_0 = min(normalized_0, normalized_1);
        auto test_1 = max(normalized_0, normalized_1);

        auto intersect_ray = [=](V2f r0, V2f r1) {
            auto ts = V2f();
            if(find_lines_intersection(r0, r1, test_0, test_1, &ts, tolerance) == false) {
                return false;
            }

            // NOTE: Use half open interavl [0; 1) to avoid overlap.  Tolerance
            // does not make sense here, as it would just offset the cut-off
            // line.
            // What is important however is that the path does not contain holes
            // between consecutive curves. (Splitting should be fine: always
            // have t = 0, t = 1 and each t1 is the t0 of another segment.)
            auto hit = in_interval_left_inclusive(ts, 0.f, 1.f);
            return hit[0] && hit[1];
        };

        fragment.winding_sign = (Sint32)sign(p1.y - p0.y, 1e-4f); // "tolerance" seems to be too strict.
        fragment.out_mask     = intersect_ray(V2f(0.f, 0.5f), V2f(1.f, 0.5f));
        fragment.sample_mask  = intersect_ray(V2f(0.f, 0.5f), V2f(0.5f, 0.5f));
    }
}


// sort by y then x.
void sort_boundary_fragments(List<Boundary_Fragment>& fragments) {
    struct Compare_Boundary_Fragments {
        Bool operator()(const Boundary_Fragment& a, const Boundary_Fragment& b) {
            if(a.y() != b.y()) { return a.y() < b.y(); }
            else               { return a.x() < b.x(); }
        }
    };
    std::sort(fragments.begin(), fragments.end(), Compare_Boundary_Fragments());
}


template <typename On_Span, typename On_Pixel>
void rasterize(
    const List<Boundary_Fragment>& fragments,
    const On_Span& on_span,
    const On_Pixel& on_pixel
) {
    auto scan_winding = Sint32();
    auto scan_line    = Sint32();
    auto scan_x       = Sint32();

    auto i = Uint(0);
    while(i < fragments.size()) {
        auto position = fragments[i].position;

        if(position.y != scan_line) {
            //assert(scan_winding == 0);
            scan_winding = 0;
            scan_line = fragments[i].y();
        }
        else if(position.x > scan_x && scan_winding != 0) {
            // Output solid span.
            auto x0 = scan_x;
            auto x1 = position.x; // exclusive.
            auto y  = scan_line;
            on_span(x0, x1, y);
        }


        // Accumulate winding changes for this pixel.
        auto delta_out_winding    = Sint32(0);
        auto delta_sample_winding = Sint32(0);
        while(i < fragments.size() && fragments[i].x() == position.x) {
            auto sign = fragments[i].winding_sign;
            delta_out_winding    += sign*fragments[i].out_mask;
            delta_sample_winding += sign*fragments[i].sample_mask;
            i += 1;
        }

        auto sample_winding = scan_winding + delta_sample_winding;
        if(sample_winding != 0) {
            auto x = position.x;
            auto y = position.y;
            on_pixel(x, y);
        }

        scan_winding    += delta_out_winding;
        scan_x           = position.x + 1;
    }
}


template <typename On_Span, typename On_Pixel>
void rasterize(
    const List<Generic_Bezier>& path,
    Float32 tolerance,
    const On_Span& on_span,
    const On_Pixel& on_pixel
) {
    auto curve_cuts = find_monotone_segments(path, tolerance);
    auto fragments = find_boundary_fragments(path, curve_cuts, tolerance);
    compute_winding(fragments, path, tolerance);
    sort_boundary_fragments(fragments);
    rasterize(fragments, on_span, on_pixel);
}


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


int main() {

    #if 1
    auto path = List<Generic_Bezier> {
        generic_line({10.f, 10.f}, {20.f, 10.f}),
        generic_quadratic({20.f, 10.f}, {23.5f, 15.f}, {30.f, 10.f}),
        generic_line({30.f, 10.f}, {37.5f, 15.f}),
        generic_cubic({37.5f, 15.f}, {28.f, 30.f}, {10.f, 22.f}, {10.f, 10.f}),
    };

    path = {
        generic_cubic({50, 50}, {100, 50}, {150, 50}, {135, 65}),
    };

    path = {
        generic_cubic({125, 325}, {150, 425}, {300, 400}, {300, 300}),
        generic_cubic({300, 300}, {300, 200}, {150, 175}, {125, 275}),
        generic_cubic({125, 275}, {125, 225}, {150, 125}, {225, 125}),
        generic_line({225, 125}, {475, 125}),
        generic_cubic({475, 125}, {400, 200}, {450, 275}, {375, 300}),
        generic_cubic({375, 300}, {450, 325}, {475, 351.2f}, {475, 400}),
        generic_cubic({475, 400}, {475, 450}, {450, 475}, {400, 475}),
        generic_line({400, 475}, {225, 475}),
        generic_cubic({225, 475}, {150, 475}, {125, 375}, {125, 325}),
    };

    path = compute_stroke(path, true, 5.f, 5.f, 0.1f);
    #endif


    constexpr Float32 tolerance = 1e-6f;

    // r, g, b, a.
    auto width  = Uint(600);
    auto height = Uint(600);
    auto stride = width*4;
    auto image = List<Uint8>(stride*height);

    auto write = [&](Sint32 x, Sint32 y, Float32 r, Float32 g, Float32 b, Float32 a) {
        if(x >= 0 && x < width && y >= 0 && y < height) {
            image[y*stride + 4*x + 0] = Uint8(r*255.0f);
            image[y*stride + 4*x + 1] = Uint8(g*255.0f);
            image[y*stride + 4*x + 2] = Uint8(b*255.0f);
            image[y*stride + 4*x + 3] = Uint8(a*255.0f);
        }
    };

    auto draw_path = [&](const List<Generic_Bezier>& path, Float32 r, Float32 g, Float32 b, Float32 a) {
        rasterize(path, tolerance,
            [&](Sint32 x0, Sint32 x1, Sint32 y) {
                for(auto x : Range<Sint32>(x0, x1)) {
                    write(x, y, r, g, b, a);
                }
            },
            [&](Sint32 x, Sint32 y) {
                write(x, y, r, g, b, a);
            }
        );
    };

    draw_path(path, 1.0f, 1.0f, 1.0f, 1.0f);

    stbi_write_png("out.png", (int)width, (int)height, 4, image.data(), (int)stride);

    return 0;
}
