#ifndef MATH_H
#define MATH_H

#include <iostream>
#include <cmath>

template <typename T>
T CLAMP(const T& x, const T& minv, const T& maxv)
{
    if(x < minv) return minv;
    if(x > maxv) return maxv;
    else         return x;
}

struct vec2 {
    float x{}, y{};
    vec2() : x(0.), y(0.) {}
    vec2(float __x, float __y) : x(__x), y(__y) {}

    vec2 operator+(const vec2& o) { return vec2(x + o.x, y + o.y); }
    void operator+=(const vec2& o) { x += o.x; y += o.y; }
    vec2 operator*(const float& f) { return vec2(x * f, y * f); }
    void operator*=(const float& f) { x *= f; y *= f; }
    vec2 operator-(const vec2& o) { return vec2(x - o.x, y - o.y); }
    void operator-=(const vec2& o) { x -= o.x; y -= o.y; }

    float length() { return sqrtf(x * x + y * y); }
};

struct vec3 {
    float x, y, z;

    vec3() : x(0.), y(0.), z(0.) {}
    vec3(float __x, float __y, float __z) : x(__x), y(__y), z(__z) {}

    vec3 operator+(const vec3& o) { return vec3(x + o.x, y + o.y, z + o.z); }
    void operator+=(const vec3& o) { x += o.x; y += o.y; z += o.z; }
    vec3 operator*(const vec3& o) { return vec3(x * o.x, y * o.y, z * o.z); }
    vec3 operator*(const float& f) { return vec3(x * f, y * f, z * f); }
    void operator*=(const float& f) { x *= f; y *= f; z *= f; }
    vec3 operator-(const vec3& o) { return vec3(x - o.x, y - o.y, z - o.z); }
    void operator-=(const vec3& o) { x -= o.x; y -= o.y; z -= o.z; }

    float dot(const vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    vec3 cross(const vec3& o) { return vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x); }
    float length() { return sqrtf(x * x + y * y + z * z); }
    void normalized() { float l = length(); if(l <= 0.) return; x /= l; y /= l; z /= l; }

    friend std::ostream& operator<<(std::ostream& stream, const vec3& v)
    {
        stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return stream;
    }

    static float dot(const vec3& x, const vec3& y) { return x.dot(y); }
    static vec3 floor(const vec3& p) { return vec3((float)(int)p.x - (p.x < 0. ? 1. : 0.), (float)(int)p.y - (p.y < 0. ? 1. : 0.), (float)(int)p.z - (p.z < 0. ? 1. : 0.)); }
};

struct vec4 {
    float x, y, z, w;

    vec4() : x(0.), y(0.), z(0.), w(0.) {}
    vec4(float __x, float __y, float __z, float __w) : x(__x), y(__y), z(__z), w(__w) {}

    vec4 operator+(const vec4& o) { return vec4(x + o.x, y + o.y, z + o.z, w + o.w); }
    void operator+=(const vec4& o) { x += o.x; y += o.y; z += o.z; }
    vec4 operator*(const vec4& o) { return vec4(x * o.x, y * o.y, z * o.z, w * o.w); }
    vec4 operator*(const float& f) { return vec4(x * f, y * f, z * f, w * f); }
    void operator*=(const float& f) { x *= f; y *= f; z *= f; }
    vec4 operator-(const vec4& o) { return vec4(x - o.x, y - o.y, z - o.z, w - o.w); }
    void operator-=(const vec4& o) { x -= o.x; y -= o.y; z -= o.z; }

    float length() { return sqrtf(x * x + y * y + z * z); }
    void normalized() { float l = length(); if(l <= 0.) return; x /= l; y /= l; z /= l; }

    static vec4 floor(const vec4& p) { return vec4((float)(int)p.x - (p.x < 0. ? 1. : 0.), (float)(int)p.y - (p.y < 0. ? 1. : 0.), (float)(int)p.z - (p.z < 0. ? 1. : 0.), (float)(int)p.w - (p.w < 0. ? 1. : 0.)); }
    static vec4 fract(const vec4& p) { return vec4(p.x - (float)(int)p.x + (p.x < 0. ? 1. : 0.), p.y - (float)(int)p.y + (p.y < 0. ? 1. : 0.), p.z - (float)(int)p.z + (p.z < 0. ? 1. : 0.), p.w - (float)(int)p.w + (p.w < 0. ? 1. : 0.)); }
};


#endif // MATH_H