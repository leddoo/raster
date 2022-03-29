#pragma once

#include <lpp/core/basic.hpp>

namespace lpp {

    template <typename T>
    T min(T a, T b) {
        if(a <= b) { return a; }
        else       { return b; }
    }

    template <typename T>
    T at_most(T a, T b) {
        return min(a, b);
    }

    template <typename T>
    T max(T a, T b) {
        if(a <= b) { return b; }
        else       { return a; }
    }

    template <typename T>
    T at_least(T a, T b) {
        return max(a, b);
    }

    template <typename T>
    T clamp(T value, T low, T high) {
        return at_most(high, at_least(low, value));
    }



    template <typename T, typename Scalar>
    T lerp(T a, T b, Scalar t) {
        return (Scalar(1) - t)*a + t*b;
    }

    template <typename Scalar>
    Scalar inverse_lerp(Scalar v, Scalar a, Scalar b) {
        return (v - a) / (b - a);
    }

}

