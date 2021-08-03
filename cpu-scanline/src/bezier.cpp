#include "bezier.hpp"


List<Bezier<V2f, 2>> reduce_degree(const Bezier<V2f, 3>& bezier, Float32 tolerance) {
    auto forward = List<Bezier<V2f, 2>>();
    auto backward = List<Bezier<V2f, 2>>();

    auto get_approximation = [](const Bezier<V2f, 3>& cubic) {
        auto quadratic = Bezier<V2f, 2>();
        quadratic[0] = cubic[0];
        quadratic[1] = 0.25f*(-cubic[0] + 3.f*cubic[1] + 3.f*cubic[2] - cubic[3]);
        quadratic[2] = cubic[3];
        return quadratic;
    };

    auto current = bezier;

    while(true) {
        auto d_01 = length(0.5f*(-current[0] + 3.f*current[1] - 3.f*current[2] + current[3]));
        auto t_split = pow(tolerance/(sqrt(3.f)/18.f * d_01), 1.f/3.f);

        if(t_split < 0.5f) {
            auto left = Bezier<V2f, 3>();
            auto right = Bezier<V2f, 3>();
            split(current, t_split, &left, &current);
            split(current, 1.f - (t_split/(1.f - t_split)), &current, &right);
            forward.push_back(get_approximation(left));
            backward.push_back(get_approximation(right));
            continue;
        }
        else if(t_split < 1.0f) {
            auto left = Bezier<V2f, 3>();
            auto right = Bezier<V2f, 3>();
            split(current, 0.5f, &left, &right);
            forward.push_back(get_approximation(left));
            backward.push_back(get_approximation(right));
            break;
        }
        else {
            forward.push_back(get_approximation(current));
            break;
        }
    }

    for(auto back = backward.rbegin(); back != backward.rend(); ++back) {
        forward.push_back(*back);
    }

    return forward;
}


Bezier<V2f, 2> offset(const Bezier<V2f, 2>& bezier, Float32 delta, Float32 tolerance) {
    auto derivative = derive<Float32>(bezier);

    auto n0 = delta*rotate_ccw(normalized(derivative[0]));
    auto p0 = bezier[0] + n0;
    auto c0 = bezier[1] + n0;

    auto n1 = delta*rotate_ccw(normalized(derivative[1]));
    auto p1 = bezier[2] + n1;
    auto c1 = bezier[1] + n1;

    auto ts = V2f();
    if(find_lines_intersection(p0, c0, p1, c1, &ts, tolerance) == false) {
        // TODO: can this happen?
        assert(false);
    }

    auto c = lerp(p0, c0, ts[0]);

    auto a0 = p0 - c;
    auto a1 = p1 - c;
    if(dot(a0, a1) >= 0.0f) {
        // TODO: split curve.
    }

    return Bezier<V2f, 2>{ p0, c, p1 };

    #if 0

    // Newton raphson.
    auto t = 0.5f*(cut_t0 + cut_t1);
    for(auto i : Range<Uint>(iter_count)) {
        UNUSED(i);
        t -= evaluate(poly, t)/evaluate(derivative, t);
    }
    #endif
}
