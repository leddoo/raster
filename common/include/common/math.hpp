#pragma once


template <typename T>
T min(T a, T b) {
    return (a <= b) ? a : b;
}

template <typename T>
T max(T a, T b) {
    return (a >= b) ? a : b;
}

template <typename T>
T clamp(T a, T low, T high) {
    return max(min(a, high), low);
}

template <typename T>
T squared(T v) { return v*v; }


template <typename T, typename Scalar>
T lerp(T a, T b, Scalar t) {
    return (Scalar(1) - t)*a + t*b;
}

template <typename Scalar>
Scalar inverse_lerp(Scalar v, Scalar a, Scalar b) {
    return (v - a) / (b - a);
}



template <typename T>
T sign(T v) {
    if     (v < T(0)) { return T(-1); }
    else if(v > T(0)) { return T(1);  }
    else              { return T(0);  }
}

template <typename T>
T sign(T v, T tolerance) {
    if     (v < T(0) - tolerance) { return T(-1); }
    else if(v > T(0) + tolerance) { return T(1);  }
    else                          { return T(0);  }
}


template <typename T>
inline Bool in_interval_inclusive(T x, T a, T b, T tolerance = T(0)) {
    return (x >= a - tolerance) && (x <= b + tolerance);
}

template <typename T>
inline Bool in_interval_exclusive(T x, T a, T b, T tolerance = T(0)) {
    return (x > a + tolerance) && (x < b - tolerance);
}

template <typename T>
inline Bool in_interval_left_inclusive(T x, T a, T b, T tolerance = T(0)) {
    return (x >= a - tolerance) && (x < b - tolerance);
}

template <typename T>
inline Bool in_interval_right_inclusive(T x, T a, T b, T tolerance = T(0)) {
    return (x > a + tolerance) && (x <= b + tolerance);
}

