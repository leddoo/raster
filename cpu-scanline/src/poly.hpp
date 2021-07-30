#pragma once

#include <common/common.hpp>
#include <common/math.hpp>

template <typename T, Uint n>
struct Poly {
    Array<T, n+1> values;

    T& operator[](Uint i) { return this->values[i]; }
    const T& operator[](Uint i) const { return this->values[i]; }
};



template <typename Scalar, typename T>
T evaluate(const Poly<T, 0>& poly, Scalar t) {
    return poly[0];
}

template <typename Scalar, typename T>
T evaluate(const Poly<T, 1>& poly, Scalar t) {
    return t*poly[1] + poly[0];
}

template <typename Scalar, typename T>
T evaluate(const Poly<T, 2>& poly, Scalar t) {
    return t*(t*poly[2] + poly[1]) + poly[0];
}

template <typename Scalar, typename T>
T evaluate(const Poly<T, 3>& poly, Scalar t) {
    return t*(t*(t*poly[3] + poly[2]) + poly[1]) + poly[0];
}



template <typename Scalar>
Uint find_roots(const Poly<Scalar, 1>& poly, Scalar& r0, Scalar tolerance) {
    if(abs(poly[1]) < tolerance) {
        return 0;
    }
    else {
        r0 = -poly[0]/poly[1];
        return 1;
    }
}

template <typename Scalar>
Uint find_roots(const Poly<Scalar, 2>& poly, Scalar& r0, Scalar& r1, Scalar tolerance) {
    if(abs(poly[2]) < tolerance) {
        return find_roots(Poly<Scalar, 1>{poly[0], poly[1]}, r0, tolerance);
    }

    auto p = poly[1]/poly[2];
    auto q = poly[0]/poly[2];

    auto x0 = -p/Scalar(2);
    auto discriminant = squared(p/Scalar(2)) - q;
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



template <typename Scalar, typename T>
Poly<T, 0> derive(const Poly<T, 0>& poly) {
    return Poly<T, 0>{
        T(0),
    };
}

template <typename Scalar, typename T>
Poly<T, 0> derive(const Poly<T, 1>& poly) {
    return Poly<T, 0>{
        poly[1],
    };
}

template <typename Scalar, typename T>
Poly<T, 1> derive(const Poly<T, 2>& poly) {
    return Poly<T, 1>{
        poly[1],
        Scalar(2)*poly[2],
    };
}

template <typename Scalar, typename T>
Poly<T, 2> derive(const Poly<T, 3>& poly) {
    return Poly<T, 2>{
        poly[1],
        Scalar(2)*poly[2],
        Scalar(3)*poly[3],
    };
}

