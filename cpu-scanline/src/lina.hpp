#pragma once

#pragma warning(disable: 4201) // anonymous structs.

#include <common/common.hpp>
#include <common/math.hpp>


template <typename T>
union V2 {
    struct { T x; T y; };
    Array<T, 2> values;

    V2() : V2(T(0)) {}
    V2(T all) : V2(all, all) {}
    V2(T x, T y) : x(x), y(y)  {}

    template <typename U>
    explicit V2(const V2<U>& other) : x(T(other.x)), y(T(other.y)) {}


    T& operator[](Uint index) {
        return this->values[index];
    }

    const T& operator[](Uint index) const {
        return this->values[index];
    }
};

using V2f = V2<Float32>;
using V2s = V2<Sint32>;
using V2u = V2<Uint32>;


template <typename T> V2<T> operator+(V2<T> a, V2<T> b) { return { a.x + b.x, a.y + b.y }; }
template <typename T> V2<T> operator-(V2<T> a, V2<T> b) { return { a.x - b.x, a.y - b.y }; }
template <typename T> V2<T> operator*(V2<T> a, V2<T> b) { return { a.x * b.x, a.y * b.y }; }
template <typename T> V2<T> operator/(V2<T> a, V2<T> b) { return { a.x / b.x, a.y / b.y }; }
template <typename T> V2<T> operator*(T a, V2<T> b) { return { a * b.x, a * b.y }; }
template <typename T> V2<T> operator*(V2<T> a, T b) { return { a.x * b, a.y * b }; }
template <typename T> V2<T> operator/(V2<T> a, T b) { return { a.x / b, a.y / b }; }
template <typename T> V2<T> operator-(V2<T> a) { return { -a.x, -a.y }; }

template <typename T> V2<T> floor(V2<T> a) { return { floor(a.x), floor(a.y) }; }
template <typename T> V2<T> ceil(V2<T> a)  { return { ceil(a.x),  ceil(a.y)  }; }
template <typename T> V2<T> abs(V2<T> a)   { return { abs(a.x),   abs(a.y)   }; }
template <typename T> V2<T> sign(V2<T> a)  { return { sign(a.x),  sign(a.y)  }; }
template <typename T> V2<T> min(V2<T> a, V2<T> b) { return { min(a.x, b.x), min(a.y, b.y) }; }
template <typename T> V2<T> max(V2<T> a, V2<T> b) { return { max(a.x, b.x), max(a.y, b.y) }; }


template <typename T> T dot(V2<T> a, V2<T> b) { return a.x*b.x + a.y*b.y; }
template <typename T> T length_squared(V2<T> a) { return dot(a, a); }
template <typename T> T length(V2<T> a) { return sqrt(length_squared(a)); }

template <typename T> V2<T> normalized(V2<T> a) { return (T(1)/length(a)) * a; }
template <typename T> V2<T> rotate_ccw(V2<T> a) { return V2<T>(-a.y, a.x); }
template <typename T> V2<T> rotate_cws(V2<T> a) { return V2<T>(a.y, -a.x); }

template <typename T> V2<Bool> operator< (V2<T> a, V2<T> b) { return V2<Bool>{ a.x <  b.x, a.y <  b.y }; }
template <typename T> V2<Bool> operator<=(V2<T> a, V2<T> b) { return V2<Bool>{ a.x <= b.x, a.y <= b.y }; }
template <typename T> V2<Bool> operator> (V2<T> a, V2<T> b) { return V2<Bool>{ a.x >  b.x, a.y >  b.y }; }
template <typename T> V2<Bool> operator>=(V2<T> a, V2<T> b) { return V2<Bool>{ a.x >= b.x, a.y >= b.y }; }


template <typename T>
V2<Bool> in_interval_left_inclusive(V2<T> v, T a, T b, T tolerance = T(0)) {
    return V2<Bool>{
        in_interval_left_inclusive(v[0], a, b, tolerance),
        in_interval_left_inclusive(v[1], a, b, tolerance),
    };
}


inline Bool any(V2<Bool> v) { return v.x || v.y; }
inline Bool all(V2<Bool> v) { return v.x && v.y; }
inline Bool none(V2<Bool> v) { return any(v) == false; }



template <typename T>
struct M2 {
    T values[2][2];

    M2() : M2(T(0)) {}
    M2(T all) : M2(all, all, all, all) {}
    M2(T m00, T m01, T m10, T m11) : values{{m00, m01}, {m10, m11}} {}

    static M2<T> from_rows(V2f r0, V2f r1)    { return M2(r0.x, r0.y, r1.x, r1.y); }
    static M2<T> from_columns(V2f c0, V2f c1) { return M2(c0.x, c1.x, c0.y, c1.y); }
};

using M2f = M2<Float32>;

template <typename T>
V2<T> operator*(M2<T> m, V2<T> v) {
    return V2<T>(
        m.values[0][0]*v.x + m.values[0][1]*v.y,
        m.values[1][0]*v.x + m.values[1][1]*v.y
    );
}


Bool invert_matrix2(M2f matrix, M2f* inverse, Float32 tolerance);


Bool find_lines_intersection(V2f a0, V2f a1, V2f b0, V2f b1, V2f* ts, Float32 tolerance);
