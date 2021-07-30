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



template <typename T>
T sign(T v) {
    if     (v < T(0)) { return T(-1); }
    else if(v > T(0)) { return T(1);  }
    else              { return T(0);  }
}

inline
Bool in_interval(Float32 x, Float32 a, Float32 b, Float32 tolerance) {
    return (x > a - tolerance) && (x < b + tolerance);
}

