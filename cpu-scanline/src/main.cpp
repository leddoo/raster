#define _CRT_SECURE_NO_WARNINGS
#include <common/common.hpp>
#include <common/math.hpp>
#include <common/slice.hpp>

#include <exception>
using Exception = std::exception;

#include "lina.hpp"
#include "poly.hpp"
#include "bezier.hpp"


#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg/nanovg.h>
#include <nanovg/nanovg_gl.h>

extern "C" {
    #include <microui/microui.h>
}




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


V2f evaluate_bernstein(const Generic_Bezier& curve, Float32 t) {
    switch (curve.degree) {
        case 0: { return evaluate_bernstein(get_bezier_0(curve), t); } break;
        case 1: { return evaluate_bernstein(get_bezier_1(curve), t); } break;
        case 2: { return evaluate_bernstein(get_bezier_2(curve), t); } break;
        case 3: { return evaluate_bernstein(get_bezier_3(curve), t); } break;

        default: throw Exception();
    }
}

V2f evaluate_casteljau(const Generic_Bezier& curve, Float32 t) {
    switch (curve.degree) {
        case 0: { return evaluate_casteljau(get_bezier_0(curve), t); } break;
        case 1: { return evaluate_casteljau(get_bezier_1(curve), t); } break;
        case 2: { return evaluate_casteljau(get_bezier_2(curve), t); } break;
        case 3: { return evaluate_casteljau(get_bezier_3(curve), t); } break;

        default: throw Exception();
    }
}

V2f evaluate(const Generic_Bezier& curve, Float32 t) {
    return evaluate_casteljau(curve, t);
}


constexpr Float32 zero_tolerance = 8*FLT_EPSILON;



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
            root_count = find_roots(poly, t0, zero_tolerance);
        } break;

        case 3: {
            auto bezier = get_bezier_3(curve);
            auto component_bezier = get_component_bezier(bezier, axis);
            auto derivative = derive<Float32>(component_bezier);
            auto poly = get_poly<Float32>(derivative);
            root_count = find_roots(poly, t0, t1, zero_tolerance);
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

void print(const Bezier<V2f, 3>& bezier) {
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

void print(const Generic_Bezier& curve) {
    switch(curve.degree) {
        case 1: print(get_bezier_1(curve)); break;
        case 2: print(get_bezier_2(curve)); break;
        case 3: print(get_bezier_3(curve)); break;
        default: assert(false);
    }
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

Array<Cut, max_cut_count> find_monotone_segments(const Generic_Bezier& curve) {
    auto cuts = Array<Cut, max_cut_count>();

    for(auto axis : Range<Uint32>(2)) {
        auto cuts_begin = axis*max_cuts_per_axis;

        auto& cut_0 = cuts[cuts_begin + 0];
        auto& cut_1 = cuts[cuts_begin + 1];
        // Initialize cuts to make sorting work.
        cut_0.t    = cut_1.t    = 1.f;
        cut_0.axis = cut_1.axis = axis;

        find_derivative_roots(curve, axis, cut_0.t, cut_1.t);
    }

    // Sort cuts by t. (We have to sort the entire array. Eg: [1, 1, 0.5, 0.75])
    insertion_sort(make_slice(cuts), [](const auto& a, const auto& b) { return a.t <= b.t; });

    return cuts;
}


List<Array<Cut, max_cut_count>> find_monotone_segments(const List<Generic_Bezier>& path) {
    auto curve_cuts = List<Array<Cut, max_cut_count>>(path.size());
    for(auto curve_index : range_of(path)) {
        curve_cuts[curve_index] = find_monotone_segments(path[curve_index]);
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
    const List<Array<Cut, max_cut_count>>& curve_cuts
) {
    auto fragments = List<Boundary_Fragment>();

    // Find boundary fragments by rasterizing the curves.
    for(auto curve_index : range_of(path)) {
        auto& curve = path[curve_index];
        auto& cuts  = curve_cuts[curve_index];

        // Note: cuts are sorted by t.

        // Skip to first positive cut. (This may skip all cuts.)
        auto cut_cursor = Uint(0);
        while(cut_cursor < max_cut_count && cuts[cut_cursor].t <= 0.f + zero_tolerance) {
            cut_cursor += 1;
        }

        // Process segments.
        auto cut_t0 = 0.f;
        while(cut_t0 < 1.f) {

            auto cut_t1 = 1.f;
            if(cut_cursor < max_cut_count && cuts[cut_cursor].t < 1.f - zero_tolerance) {
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

                // segment is monotonic -> at most one root in segment interval.
                // root finding may however return roots outside of the segment.

                auto clamp_t = [=](Float32 t) {
                    if     (t < cut_t0 - zero_tolerance) { return 2.f; }
                    else if(t > cut_t1 + zero_tolerance) { return 2.f; }
                    return clamp(t, cut_t0, cut_t1);
                };

                auto moved = curve;
                for(auto& point : moved.values) {
                    point[axis] -= next_pos;
                }

                // early out if there is no root.
                auto y0 = evaluate(moved, cut_t0)[axis];
                auto y1 = evaluate(moved, cut_t1)[axis];
                if(float_sign(y0) == float_sign(y1)) {
                    return 2.0f;
                }

                auto t_min = 2.f;

                switch (moved.degree) {
                    case 1: {
                        auto bezier = get_bezier_1(moved);
                        auto component_bezier = get_component_bezier(bezier, axis);
                        auto poly = get_poly<Float32>(component_bezier);

                        auto r0 = 2.f;
                        find_roots(poly, r0, zero_tolerance);
                        t_min = clamp_t(r0);
                    } break;

                    case 2: {
                        auto bezier = get_bezier_2(moved);
                        auto component_bezier = get_component_bezier(bezier, axis);
                        auto poly = get_poly<Float32>(component_bezier);

                        auto r0 = 2.f, r1 = 2.f;
                        find_roots(poly, r0, r1, zero_tolerance);
                        t_min = min(clamp_t(r0), clamp_t(r1));
                    } break;

                    case 3: {
                        constexpr Uint iter_count = 8;

                        auto bezier = get_bezier_3(moved);
                        auto component_bezier = get_component_bezier(bezier, axis);
                        auto poly = get_poly<Float32>(component_bezier);

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
                    if(t_min <= 1.f + zero_tolerance) {
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
                    cut_t0 = min(step_t, cut_t1);

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
    const List<Generic_Bezier>& path
) {
    // compute per fragment winding changes and sample mask.
    for(auto i : range_of(fragments)) {
        auto& fragment = fragments[i];

        auto t0 = fragments[i].t0;
        auto t1 = 1.f;
        if(i + 1 < fragments.size() && fragment.curve_index == fragments[i + 1].curve_index) {
            t1 = fragments[i + 1].t0;
        }

        // treated as exact.
        auto p0 = evaluate(path[fragments[i].curve_index], t0);
        auto p1 = evaluate(path[fragments[i].curve_index], t1);

        auto normalized_0 = p0 - V2f(fragment.position);
        auto normalized_1 = p1 - V2f(fragment.position);

        // Sort coordinates to make hit testing curve direction independent.
        auto test_0 = min(normalized_0, normalized_1);
        auto test_1 = max(normalized_0, normalized_1);

        auto intersect_ray = [=](V2f r0, V2f r1) {
            auto ts = V2f();
            // TODO: should we use a tolerance here?
            if(find_lines_intersection(r0, r1, test_0, test_1, &ts, zero_tolerance) == false) {
                return false;
            }

            // NOTE: Use half open interavl [0; 1) to avoid overlap.  Tolerance
            // does not make sense here, as it would just offset the cut-off
            // line.
            // What is important however is that the path does not contain holes
            // between consecutive curves. (Splitting should be fine: always
            // have t = 0, t = 1 and each t1 is the t0 of another segment.)
            return all(in_interval_left_inclusive(ts, 0.f, 1.f));
        };

        fragment.winding_sign = (Sint32)sign(p1.y - p0.y);
        fragment.out_mask     = intersect_ray(V2f(1.0f, 0.5f), V2f(0.0f, 0.5f));
        fragment.sample_mask  = intersect_ray(V2f(0.5f, 0.5f), V2f(0.0f, 0.5f));
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

auto problem_lines = List<V2s>();

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
            if(scan_winding != 0) {
                problem_lines.push_back({scan_x, scan_line});
            }
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
        while(i < fragments.size() && all(fragments[i].position == position)) {
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
    const On_Span& on_span,
    const On_Pixel& on_pixel
) {
    auto curve_cuts = find_monotone_segments(path);
    auto fragments = find_boundary_fragments(path, curve_cuts);
    compute_winding(fragments, path);
    sort_boundary_fragments(fragments);
    rasterize(fragments, on_span, on_pixel);
}


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


struct Path {
    List<Generic_Bezier> curves;
    Bool closed;
};

struct Color {
    Float32 r, g, b, a;
};

struct Tiger_Path {
    Path path;
    Color fill;
    Color stroke;
    Float32 stroke_width;
};


extern List<Tiger_Path> tiger;


static void glfw_error(int error, const char* desc) {
    printf("GLFW error %d: %s\n", error, desc);
}


auto mouse_position  = V2f(0.f, 0.f);

auto camera_position = V2f(0.f, 0.f);
auto camera_scale    = 1.f;
constexpr auto camera_scale_min = 0.1f;
constexpr auto camera_scale_max = 256.0f;
constexpr auto camera_scale_delta = 0.01f;

V2f to_world(V2f s) {
    return (1.0f/camera_scale)*s + camera_position;
}

V2f to_screen(V2f w) {
    return camera_scale*(w - camera_position);
}


auto nvg = (NVGcontext*)nullptr;
auto mui = (mu_Context*)nullptr;


static void on_key(GLFWwindow* window, int key, int scancode, int action, int modifiers) {
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(modifiers);

    auto mu_key = 0;
    if     (key == GLFW_KEY_LEFT_SHIFT)    { mu_key = MU_KEY_SHIFT; }
    else if(key == GLFW_KEY_RIGHT_SHIFT)   { mu_key = MU_KEY_SHIFT; }
    else if(key == GLFW_KEY_LEFT_CONTROL)  { mu_key = MU_KEY_CTRL; }
    else if(key == GLFW_KEY_RIGHT_CONTROL) { mu_key = MU_KEY_CTRL; }
    else if(key == GLFW_KEY_LEFT_ALT)      { mu_key = MU_KEY_ALT; }
    else if(key == GLFW_KEY_RIGHT_ALT)     { mu_key = MU_KEY_ALT; }
    else if(key == GLFW_KEY_ENTER)         { mu_key = MU_KEY_RETURN; }
    else if(key == GLFW_KEY_BACKSPACE)     { mu_key = MU_KEY_BACKSPACE; }

    if(action == GLFW_PRESS || action == GLFW_REPEAT) {
        mu_input_keydown(mui, mu_key);
    }
    else if(action == GLFW_RELEASE) {
        mu_input_keyup(mui, mu_key);
    }
}

static void on_char(GLFWwindow* window, unsigned int code_point) {
    UNUSED(window);

    const char text[] = { char(code_point), 0 };
    mu_input_text(mui, text);
}

static void on_mouse_button(GLFWwindow* window, int button, int action, int modifiers) {
    UNUSED(modifiers);

    auto x = double(), y = double();
    glfwGetCursorPos(window, &x, &y);

    auto mu_button = 0;
    if     (button == GLFW_MOUSE_BUTTON_LEFT) { mu_button = MU_MOUSE_LEFT; }
    else if(button == GLFW_MOUSE_BUTTON_RIGHT) { mu_button = MU_MOUSE_RIGHT; }
    else if(button == GLFW_MOUSE_BUTTON_MIDDLE) { mu_button = MU_MOUSE_MIDDLE; }

    if(action == GLFW_PRESS) {
        mu_input_mousedown(mui, int(x), int(y), mu_button);
    }
    else if(action == GLFW_RELEASE) {
        mu_input_mouseup(mui, int(x), int(y), mu_button);
    }
}

static void on_mouse_position(GLFWwindow* window, double x, double y) {
    mu_input_mousemove(mui, int(x), int(y));

    auto new_position = V2f(Float32(x), Float32(y));
    defer { mouse_position = new_position; };

    auto delta = new_position - mouse_position;
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
        camera_position = camera_position - (1.f/camera_scale)*delta;
    }
}

static void on_mouse_scroll(GLFWwindow* window, double dx, double dy) {
    UNUSED(window);
    UNUSED(dx);

    auto scale = [](Float32 t) {
        return camera_scale_min + (exp2(8.f*t) - 1.0f)/255.0f * (camera_scale_max - camera_scale_min);
    };
    auto scale_inv = [](Float32 scale) {
        return log2((scale - camera_scale_min) * 255.0f / (camera_scale_max - camera_scale_min) + 1.0f) / 8.0f;
    };

    auto t = clamp(scale_inv(camera_scale) + camera_scale_delta*Float32(dy), 0.0f, 1.0f);
    auto new_scale = scale(t);
    defer { camera_scale = new_scale; };

    camera_position = camera_position + mouse_position/camera_scale - mouse_position/new_scale;
}



struct Rect {
    V2f min;
    V2f max;
};

Bool intersect(const Rect& a, const Rect& b) {
    return none(a.min >= b.max) && none(b.min >= a.max);
}

Rect get_union(const Rect& a, const Rect& b) {
    return Rect {
        min(a.min, b.min),
        max(a.max, b.max),
    };
}

Rect compute_aabb(const List<Generic_Bezier>& path) {
    auto result = Rect {
        V2f(+FLT_MAX, +FLT_MAX),
        V2f(-FLT_MAX, -FLT_MAX),
    };

    for(const auto& curve : path) {
        for(auto i : Range<Uint>(curve.degree + 1)) {
            result.min = min(result.min, curve[i]);
            result.max = max(result.max, curve[i]);
        }
    }

    return result;
}



int main() {

    #if 0
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

    auto tiger_aabb = Rect{ V2f(+FLT_MAX, +FLT_MAX), V2f(-FLT_MAX, -FLT_MAX) };
    auto path_aabbs = List<Rect>(tiger.size());

    for(auto i : range_of(tiger)) {
        auto aabb = compute_aabb(tiger[i].path.curves);
        path_aabbs[i] = aabb;
        tiger_aabb = get_union(tiger_aabb, aabb);
    }


    auto padding    = V2f(20.0f, 20.0f);
    auto offset     = -tiger_aabb.min + padding;
    auto image_size = ceil(tiger_aabb.max - tiger_aabb.min + 2.f*padding);

    for(auto i : range_of(tiger)) {
        for(auto& curve : tiger[i].path.curves) {
            for(auto i : Range<Uint>(curve.degree + 1)) {
                curve[i] = curve[i] + offset;
            }
        }
        path_aabbs[i].min = path_aabbs[i].min + offset;
        path_aabbs[i].max = path_aabbs[i].max + offset;
    }

    // r, g, b, a.
    auto stride = Uint(4*image_size.x);
    auto image = List<Uint8>(Uint(stride*image_size.y));

    auto write = [&](Sint32 x, Sint32 y, Color c) {
        if(    x >= 0 && x < Sint32(image_size.x)
            && y >= 0 && y < Sint32(image_size.y)
        ) {
            image[y*stride + 4*x + 0] = Uint8(c.r*255.0f);
            image[y*stride + 4*x + 1] = Uint8(c.g*255.0f);
            image[y*stride + 4*x + 2] = Uint8(c.b*255.0f);
            image[y*stride + 4*x + 3] = Uint8(c.a*255.0f);
        }
    };


    constexpr Float32 tolerance = 1e-6f;

    auto draw_path = [&](const List<Generic_Bezier>& path, Color color) {
        rasterize(path,
            [&](Sint32 x0, Sint32 x1, Sint32 y) {
                for(auto x : Range<Sint32>(x0, x1)) {
                    write(x, y, color);
                }
            },
            [&](Sint32 x, Sint32 y) {
                write(x, y, color);
            }
        );
    };


    for(const auto& path : tiger) {
        if(path.fill.a > 0.f) {
            draw_path(path.path.curves, path.fill);
        }
    }

    printf("%lld\n", problem_lines.size());

    #if 0
    {
        auto p = tiger[19];
        const auto& path = p.path.curves;
        auto curve_cuts = find_monotone_segments(path);
        auto fragments = find_boundary_fragments(path, curve_cuts);
        compute_winding(fragments, path);
        sort_boundary_fragments(fragments);
        rasterize(fragments, [](Sint32 x0, Sint32 x1, Sint32 y){}, [](Sint32, Sint32){});

        for(auto curve : path) {
            print(curve);
        }
        printf("\n\n");

        for(auto f : fragments) {
            auto x = f.position.x;
            auto y = f.position.y;
            printf("polygon((%d, %d), (%d, %d), (%d, %d), (%d, %d)), ",
                x, y, x + 1, y, x + 1, y + 1, x, y + 1
            );
        }
        printf("\n\n");

        //if(true) return 0;
    }
    #endif


    auto glfw_init_result = glfwInit();
    assert(glfw_init_result);

    glfwSetErrorCallback(glfw_error);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    glfwWindowHint(GLFW_SAMPLES, 32);
    auto window = glfwCreateWindow(800, 600, "cpu-scanline", nullptr, nullptr);
    assert(window != nullptr);

    glfwSetKeyCallback(window, on_key);
    glfwSetCharCallback(window, on_char);
    glfwSetMouseButtonCallback(window, on_mouse_button);
    glfwSetCursorPosCallback(window, on_mouse_position);
    glfwSetScrollCallback(window, on_mouse_scroll);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);


    auto glew_init_result = glewInit();
    assert(glew_init_result == GLEW_OK);
    glGetError(); // swallow some glew error.


    nvg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
    assert(nvg != nullptr);


    struct Nvg_Font {
        Sint32      font;
        Float32     size;
    };

    auto font = Nvg_Font {
        nvgCreateFont(nvg, "Roboto", "Roboto-Regular.ttf"),
        18.0f,
    };


    mui = new mu_Context();
    mu_init(mui);
    mui->style->font = &font;

    mui->text_width = [](mu_Font font, const char* string, int length) -> int {
        UNUSED(length);

        auto nvg_font = (Nvg_Font*)font;
        auto aabb = Rect();
        nvgTextLineHeight(nvg, nvg_font->size);
        return int(nvgTextBounds(nvg, 0, 0, string, nullptr, nullptr));
    };

    mui->text_height = [](mu_Font font) -> int {
        auto nvg_font = (Nvg_Font*)font;
        return int(nvg_font->size);
    };


    auto my_image = nvgCreateImageRGBA(nvg, int(image_size.x), int(image_size.y), NVG_IMAGE_NEAREST, image.data());
    assert(my_image != 0);


    struct Path_Data {
        Color color;
        Bool  open;
    };

    auto path_datas = List<Path_Data>(tiger.size());
    for(auto& data : path_datas) {
        data.color.r = Float32(rand() % 255) / 255.0f;
        data.color.g = Float32(rand() % 255) / 255.0f;
        data.color.b = Float32(rand() % 255) / 255.0f;
    }


    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();


        auto fb_width = 0;
        auto fb_height = 0;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);

        auto fb_size = V2f(Float32(fb_width), Float32(fb_height));
        auto screen_rect = Rect { to_world(V2f(0.0f, 0.0f)), to_world(fb_size) };


        auto visible_paths = List<Uint>();
        for(auto i : range_of(tiger)) {
            if(intersect(screen_rect, path_aabbs[i])){
                visible_paths.push_back(i);
            }
        }

        auto hovered_path = Uint(-1);


        mu_begin(mui);

        // visible paths window.
        if(mu_begin_window_ex(mui, "Visible Paths", (mu_Rect { 20, 20, 200, 300 }), MU_OPT_NOCLOSE)) {
            defer { mu_end_window(mui); };

            // button: detect hover, open window on click.
            for(auto path_index : visible_paths) {
                auto id = mu_get_id(mui, &path_index, sizeof(path_index));
                auto rect = mu_layout_next(mui);
                mu_update_control(mui, id, rect, 0);

                if(mui->hover == id) {
                    hovered_path = path_index;
                }
                if(mui->focus == id && (mui->mouse_pressed & MU_MOUSE_LEFT) != 0) {
                    path_datas[path_index].open = true;
                }

                char buffer[32];
                sprintf_s(buffer, sizeof(buffer), "%lld", path_index);
                mu_draw_control_frame(mui, id, rect, MU_COLOR_BUTTON, 0);
                mu_draw_control_text(mui, buffer, rect, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
            }
        }

        // path windows.
        for(auto path_index : range_of(tiger)) {
            if(path_datas[path_index].open == false) {
                continue;
            }

            char title[32];
            sprintf_s(title, sizeof(title), "Path %lld", path_index);

            // sync open state.
            auto container = mu_get_container(mui, title);
            if(container != nullptr) { container->open = true; }

            // header color.
            auto bg_color = &mui->style->colors[MU_COLOR_TITLEBG];
            auto old_bg_color = *bg_color;
            defer { *bg_color = old_bg_color; };
            auto color = path_datas[path_index].color;
            *bg_color = mu_Color { Uint8(color.r*255.0f), Uint8(color.g*255.0f), Uint8(color.b*255.0f), 255 };

            if(mu_begin_window_ex(mui, title, (mu_Rect { 300, 100, 200, 300 }), 0)) {
                defer { mu_end_window(mui); };

                auto fill = tiger[path_index].fill;
                auto mu_fill = mu_Color { Uint8(fill.r*255.0f), Uint8(fill.g*255.0f), Uint8(fill.b*255.0f), Uint8(fill.a*255.0f) };
                char fill_text[32];
                sprintf_s(fill_text, sizeof(fill_text), "fill: %d, %d, %d, %d", mu_fill.r, mu_fill.g, mu_fill.b, mu_fill.a);

                int widths = -1;
                mu_layout_row(mui, 1, &widths, 0);
                mu_label(mui, fill_text);
                mu_draw_rect(mui, mu_layout_next(mui), mu_fill);
            }

            // sync open state.
            container = mu_get_container(mui, title);
            if(container != nullptr) { path_datas[path_index].open = container->open; }
        }


        mu_end(mui);



        // draw.
        glViewport(0, 0, fb_width, fb_height);
        glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        nvgBeginFrame(nvg, fb_size.x, fb_size.y, 1.0f);


        nvgSave(nvg);

        // this is translate then scale, I think. "pre-multiply"
        nvgScale(nvg, camera_scale, camera_scale);
        nvgTranslate(nvg, -camera_position.x, -camera_position.y);

        // Rasterized image.
        {
            auto image_paint = nvgImagePattern(nvg, 0.0f, 0.0f, image_size.x, image_size.y, 0.0f, my_image, 1.0f);
            nvgBeginPath(nvg);
            nvgRect(nvg, 0, 0, image_size.x, image_size.y);
            nvgFillPaint(nvg, image_paint);
            nvgFill(nvg);
        }

        // Grid lines.
        if(camera_scale >= 3.0f) {
            auto alpha = min(inverse_lerp(camera_scale, 3.0f, 5.0f), 1.0f);
            auto begin = floor(screen_rect.min);
            auto end   = ceil(screen_rect.max);
            nvgBeginPath(nvg);
            for(auto x : Range<Float32>(begin.x, end.x)) {
                nvgMoveTo(nvg, x, begin.y);
                nvgLineTo(nvg, x, end.y);
            }
            for(auto y : Range<Float32>(begin.y, end.y)) {
                nvgMoveTo(nvg, begin.x, y);
                nvgLineTo(nvg, end.x,   y);
            }
            nvgStrokeColor(nvg, NVGcolor { 0.0f, 1.0f, 1.0f, 0.3f*alpha });
            nvgStrokeWidth(nvg, 1.0f/camera_scale);
            nvgStroke(nvg);
        }

        {
            nvgBeginPath(nvg);
            for(auto line : problem_lines) {
                nvgMoveTo(nvg, line.x    , line.y);
                nvgLineTo(nvg, line.x + 1, line.y);
                nvgLineTo(nvg, line.x + 1, line.y + 1);
                nvgLineTo(nvg, line.x    , line.y + 1);
                nvgClosePath(nvg);
            }
            nvgStrokeColor(nvg, NVGcolor { 0.1f, 0.4f, 0.8f, 1.0f });
            nvgStrokeWidth(nvg, 3.0f/camera_scale);
            nvgStroke(nvg);
            nvgStrokeColor(nvg, NVGcolor { 0.8f, 0.4f, 0.1f, 1.0f });
            nvgStrokeWidth(nvg, 1.0f/camera_scale);
            nvgStroke(nvg);
        }

        // highlight paths.
        {
            for(auto path_index : visible_paths) {
                const auto& path = tiger[path_index];
                const auto& data = path_datas[path_index];
                nvgBeginPath(nvg);
                nvgMoveTo(nvg, path.path.curves[0][0].x, path.path.curves[0][0].y);

                for(auto& curve : path.path.curves) {
                    switch(curve.degree) {
                        case 1: {
                            nvgLineTo(nvg, curve[1].x, curve[1].y);
                        } break;
                        case 2: {
                            nvgQuadTo(nvg, curve[1].x, curve[1].y, curve[2].x, curve[2].y);
                        } break;
                        case 3: {
                            nvgBezierTo(nvg, curve[1].x, curve[1].y, curve[2].x, curve[2].y, curve[3].x, curve[3].y);
                        } break;
                        default: assert(false);
                    }
                }

                const auto highlight_color = NVGcolor { 0.7f, 0.8f, 1.0f, 1.0f };

                auto color = NVGcolor { 1.0f, 0.0f, 1.0f, 0.75f };
                auto stroke_width = 1.0f;

                if(path_index == hovered_path || data.open) {
                    color = highlight_color;
                    stroke_width = 5.0f;
                }

                nvgStrokeColor(nvg, color);
                nvgStrokeWidth(nvg, stroke_width/camera_scale);
                nvgStroke(nvg);

                // inner stroke with path color.
                if(data.open) {
                    auto inner_color = NVGcolor {
                        color.r = data.color.r,
                        color.g = data.color.g,
                        color.b = data.color.b,
                        0.75f
                    };
                    nvgStrokeColor(nvg, inner_color);
                    nvgStrokeWidth(nvg, 2.0f/camera_scale);
                    nvgStroke(nvg);
                }

                // control point aabb for hovered path.
                if(path_index == hovered_path) {
                    auto aabb = path_aabbs[path_index];
                    auto pos = aabb.min;
                    auto size = aabb.max - aabb.min;
                    nvgBeginPath(nvg);
                    nvgRect(nvg, pos.x, pos.y, size.x, size.y);

                    nvgStrokeColor(nvg, NVGcolor { 0.2f, 0.2f, 0.2f, 1.0f });
                    nvgStrokeWidth(nvg, 5.0f/camera_scale);
                    nvgStroke(nvg);

                    nvgStrokeColor(nvg, highlight_color);
                    nvgStrokeWidth(nvg, 3.0f/camera_scale);
                    nvgStroke(nvg);
                }
            }
        }

        nvgRestore(nvg);


        // process mu paint commands.
        auto mu_cmd = (mu_Command*)nullptr;
        while(mu_next_command(mui, &mu_cmd)) {
            if(mu_cmd->type == MU_COMMAND_RECT) {
                auto rect = mu_cmd->rect.rect;
                auto color = mu_cmd->rect.color;
                nvgBeginPath(nvg);
                nvgRect(nvg, Float32(rect.x), Float32(rect.y), Float32(rect.w), Float32(rect.h));
                nvgFillColor(nvg, NVGcolor { Float32(color.r)/255.0f, Float32(color.g)/255.0f, Float32(color.b)/255.0f, Float32(color.a)/255.0f });
                nvgFill(nvg);
            }
            else if(mu_cmd->type == MU_COMMAND_TEXT) {
                auto nvg_font = (Nvg_Font*)mu_cmd->text.font;
                auto color = mu_cmd->text.color;
                auto pos = mu_cmd->text.pos;
                nvgFontFaceId(nvg, nvg_font->font);
                nvgFontSize(nvg, nvg_font->size);
                nvgFillColor(nvg, NVGcolor { Float32(color.r)/255.0f, Float32(color.g)/255.0f, Float32(color.b)/255.0f, Float32(color.a)/255.0f });
                nvgTextAlign(nvg, NVG_ALIGN_TOP);
                nvgText(nvg, Float32(pos.x), Float32(pos.y), &mu_cmd->text.str[0], nullptr);
            }
            else if(mu_cmd->type == MU_COMMAND_CLIP) {
                auto rect = mu_cmd->clip.rect;
                nvgScissor(nvg, Float32(rect.x), Float32(rect.y), Float32(rect.w), Float32(rect.h));
            }
        }

        nvgEndFrame(nvg);
        glfwSwapBuffers(window);
    }

    nvgDeleteGL3(nvg);

    glfwTerminate();

    return 0;
}

