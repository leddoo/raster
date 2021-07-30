#pragma once

#include <common/common.hpp>
#include <common/math.hpp>

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
T evaluate(const Bezier<T, 0>& bezier, Scalar t) {
    UNUSED(t);
    return bezier[0];
}

template <typename T, typename Scalar>
T evaluate(const Bezier<T, 1>& bezier, Scalar t) {
    return
          bernstein_1_0(t)*bezier[0]
        + bernstein_1_1(t)*bezier[1];
}

template <typename T, typename Scalar>
T evaluate(const Bezier<T, 2>& bezier, Scalar t) {
    return
          bernstein_2_0(t)*bezier[0]
        + bernstein_2_1(t)*bezier[1]
        + bernstein_2_2(t)*bezier[2];
}

template <typename T, typename Scalar>
T evaluate(const Bezier<T, 3>& bezier, Scalar t) {
    return
          bernstein_3_0(t)*bezier[0]
        + bernstein_3_1(t)*bezier[1]
        + bernstein_3_2(t)*bezier[2]
        + bernstein_3_3(t)*bezier[3];
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

