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


/*
using V2f = glm::vec2;
using V3f = glm::vec3;
using V4f = glm::vec4;

inline V2f min(V2f a, V2f b) { return V2f { min(a.x, b.x), min(a.y, b.y) }; }
inline V2f max(V2f a, V2f b) { return V2f { max(a.x, b.x), max(a.y, b.y) }; }

inline Bool operator < (V2f a, V2f b) { return (a.x <  b.x) && (a.y <  b.y); }
inline Bool operator <=(V2f a, V2f b) { return (a.x <= b.x) && (a.y <= b.y); }
inline Bool operator > (V2f a, V2f b) { return (a.x >  b.x) && (a.y >  b.y); }
inline Bool operator >=(V2f a, V2f b) { return (a.x >= b.x) && (a.y >= b.y); }

inline Float32 dot(V2f a, V2f b) { return a.x*b.x + a.y*b.y; }

inline Float32 length_squared(V2f v) { return dot(v, v); }

inline V2f round(V2f a) { return V2f { round(a.x), round(a.y) }; }


inline V4f make_color(Float32 r, Float32 g, Float32 b, Float32 a = 255) {
    return V4f { r/255.0f, g/255.0f, b/255.0f, a/255.0f };
}
*/
