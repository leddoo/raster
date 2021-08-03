#pragma once

#include <common/common.hpp>
#include <common/math.hpp>
#include "lina.hpp"

template <typename T, Uint n>
struct Poly;



template <typename T, Uint n>
struct Bezier {
    Array<T, n+1> values;

    T& operator[](Uint i) { return this->values[i]; }
    const T& operator[](Uint i) const { return this->values[i]; }
};



template <typename T> T bernstein_1_0(T t) { auto x = T(1) - t; return x; }
template <typename T> T bernstein_1_1(T t) {                    return t; }

template <typename T> T bernstein_2_0(T t) { auto x = T(1) - t; return x*x;      }
template <typename T> T bernstein_2_1(T t) { auto x = T(1) - t; return T(2)*x*t; }
template <typename T> T bernstein_2_2(T t) {                    return t*t;      }

template <typename T> T bernstein_3_0(T t) { auto x = T(1) - t; return x*x*x;      }
template <typename T> T bernstein_3_1(T t) { auto x = T(1) - t; return T(3)*x*x*t; }
template <typename T> T bernstein_3_2(T t) { auto x = T(1) - t; return T(3)*x*t*t; }
template <typename T> T bernstein_3_3(T t) {                    return t*t*t;      }


template <typename T, typename Scalar>
T evaluate_bernstein(const Bezier<T, 0>& bezier, Scalar t) {
    UNUSED(t);
    return bezier[0];
}

template <typename T, typename Scalar>
T evaluate_bernstein(const Bezier<T, 1>& bezier, Scalar t) {
    return
          bernstein_1_0(t)*bezier[0]
        + bernstein_1_1(t)*bezier[1];
}

template <typename T, typename Scalar>
T evaluate_bernstein(const Bezier<T, 2>& bezier, Scalar t) {
    return
          bernstein_2_0(t)*bezier[0]
        + bernstein_2_1(t)*bezier[1]
        + bernstein_2_2(t)*bezier[2];
}

template <typename T, typename Scalar>
T evaluate_bernstein(const Bezier<T, 3>& bezier, Scalar t) {
    return
          bernstein_3_0(t)*bezier[0]
        + bernstein_3_1(t)*bezier[1]
        + bernstein_3_2(t)*bezier[2]
        + bernstein_3_3(t)*bezier[3];
}


template <typename T, Uint n, typename Scalar>
T evaluate_casteljau(const Bezier<T, n>& bezier, Scalar t) {
    auto values = bezier.values;
    for(auto i : Range<Uint>(n)) {
        for(auto j : Range<Uint>(n - i)) {
            values[j] = (Scalar(1) - t)*values[j] + t*values[j + 1];
        }
    }
    auto b = evaluate_bernstein(bezier, t);
    return values[0];
}



template <typename T>
Bezier<T, 0> get_component_bezier(const Bezier<V2<T>, 0>& bezier, Uint axis) {
    return Bezier<T, 0>{ bezier[0][axis] };
}

template <typename T>
Bezier<T, 1> get_component_bezier(const Bezier<V2<T>, 1>& bezier, Uint axis) {
    return Bezier<T, 1>{ bezier[0][axis], bezier[1][axis] };
}

template <typename T>
Bezier<T, 2> get_component_bezier(const Bezier<V2<T>, 2>& bezier, Uint axis) {
    return Bezier<T, 2>{ bezier[0][axis], bezier[1][axis], bezier[2][axis] };
}

template <typename T>
Bezier<T, 3> get_component_bezier(const Bezier<V2<T>, 3>& bezier, Uint axis) {
    return Bezier<T, 3>{ bezier[0][axis], bezier[1][axis], bezier[2][axis], bezier[3][axis] };
}



template <typename Scalar, typename T>
Poly<T, 0> get_poly(const Bezier<T, 0>& bezier) {
    return Poly<T, 0>{
        bezier[0],
    };
}

template <typename Scalar, typename T>
Poly<T, 1> get_poly(const Bezier<T, 1>& bezier) {
    return Poly<T, 1>{
        bezier[0],
        bezier[1] - bezier[0],
    };
}

template <typename Scalar, typename T>
Poly<T, 2> get_poly(const Bezier<T, 2>& bezier) {
    return Poly<T, 2>{
        bezier[0],
        Scalar(2)*(bezier[1] - bezier[0]),
        bezier[0] - Scalar(2)*bezier[1] + bezier[2],
    };
}

template <typename Scalar, typename T>
Poly<T, 3> get_poly(const Bezier<T, 3>& bezier) {
    return Poly<T, 3>{
        bezier[0],
        Scalar(3)*(bezier[1] - bezier[0]),
        Scalar(3)*(bezier[0] - Scalar(2)*bezier[1] + bezier[2]),
        -bezier[0] + Scalar(3)*bezier[1] - Scalar(3)*bezier[2] + bezier[3],
    };
}


template <typename Scalar, typename T>
Bezier<T, 2> get_bezier(const Poly<T, 2>& poly) {
    return Bezier<T, 2>{
        poly[0],
        poly[0] + poly[1]/Scalar(2),
        poly[0] + poly[1] + poly[2],
    };
}



template <typename Scalar, typename T>
Bezier<T, 0> derive(const Bezier<T, 0>& bezier) {
    return Bezier<T, 0>{
        T(0),
    };
}

template <typename Scalar, typename T>
Bezier<T, 0> derive(const Bezier<T, 1>& bezier) {
    return Bezier<T, 0>{
        (bezier[1] - bezier[0]),
    };
}

template <typename Scalar, typename T>
Bezier<T, 1> derive(const Bezier<T, 2>& bezier) {
    return Bezier<T, 1>{
        Scalar(2)*(bezier[1] - bezier[0]),
        Scalar(2)*(bezier[2] - bezier[1]),
    };
}

template <typename Scalar, typename T>
Bezier<T, 2> derive(const Bezier<T, 3>& bezier) {
    return Bezier<T, 2>{
        Scalar(3)*(bezier[1] - bezier[0]),
        Scalar(3)*(bezier[2] - bezier[1]),
        Scalar(3)*(bezier[3] - bezier[2]),
    };
}


template <typename T, Uint n>
Bezier<T, n> reverse(const Bezier<T, n>& bezier) {
    auto result = bezier;
    std::reverse(result.values.begin(), result.values.end());
    return result;
}


template <typename T, typename Scalar>
void split(const Bezier<T, 3>& bezier, Scalar t, Bezier<T, 3>* s0, Bezier<T, 3>* s1) {
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
    if(s0 != nullptr) {
        (*s0)[0] = l00;
        (*s0)[1] = l10;
        (*s0)[2] = l20;
        (*s0)[3] = l30;
    }
    if(s1 != nullptr) {
        (*s1)[0] = l30;
        (*s1)[1] = l21;
        (*s1)[2] = l12;
        (*s1)[3] = l03;
    }
}


List<Bezier<V2f, 2>> reduce_degree(const Bezier<V2f, 3>& bezier, Float32 tolerance);

Bezier<V2f, 2> offset(const Bezier<V2f, 2>& bezier, Float32 delta, Float32 tolerance);

