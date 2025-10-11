#ifndef MATH_H
#define MATH_H

#include <iostream>
#include <cmath>

template<typename T>
inline float sign(const T& x)
{
    return x > 0. ? 1. : -1.;
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
    vec3 operator*(const float& f) { return vec3(x * f, y * f, z * f); }
    void operator*=(const float& f) { x *= f; y *= f; z *= f; }

    float dot(const vec3& o) { return x * o.x + y * o.y + z * o.z; }
    vec3 cross(const vec3& o) { return vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x); }
    float length() { return sqrtf(x * x + y * y + z * z); }
    void normalized() { float l = length(); if(l <= 0.) return; x /= l; y /= l; z /= l; }

    friend std::ostream& operator<<(std::ostream& stream, const vec3& v)
    {
        stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return stream;
    }
};

#endif // MATH_H