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

    vec3 operator+(const vec3& o) const { return vec3(x + o.x, y + o.y, z + o.z); }
    void operator+=(const vec3& o) { x += o.x; y += o.y; z += o.z; }
    vec3 operator*(const vec3& o) const { return vec3(x * o.x, y * o.y, z * o.z); }
    vec3 operator*(const float& f) const { return vec3(x * f, y * f, z * f); }
    void operator*=(const float& f) { x *= f; y *= f; z *= f; }
    vec3 operator-(const vec3& o) const { return vec3(x - o.x, y - o.y, z - o.z); }
    void operator-=(const vec3& o) { x -= o.x; y -= o.y; z -= o.z; }
    float dot(const vec3& other) const { return x * other.x + y * other.y + z * other.z; }
    vec3 cross(const vec3& o) const { return vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x); }
    float length() const { return sqrtf(x * x + y * y + z * z); }
    void normalized() { float l = length(); if(l <= 0.) return; x /= l; y /= l; z /= l; }
    vec3 normalize() const { vec3 ans = *this; ans.normalized(); return ans; }

    friend std::ostream& operator<<(std::ostream& stream, const vec3& v)
    {
        stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return stream;
    }

    static float dot(const vec3& x, const vec3& y) { return x.dot(y); }
    static vec3 floor(const vec3& p) { return vec3((float)(int)p.x - (p.x < 0. ? 1. : 0.), (float)(int)p.y - (p.y < 0. ? 1. : 0.), (float)(int)p.z - (p.z < 0. ? 1. : 0.)); }

    // rotates any vector around any axis
    // I spent hours deriving this function for it not to be useful at all bruh
    vec3 rotate(vec3 axis, float angle)
    {
        vec3 up(0., 1., 0.), naxis = axis.normalize();
        float de = sin(angle), ph = cos(angle);
        if(fabsf(naxis.y) > 0.999) // rotation around (Oy), no change of basis is needed
            return vec3(dot(vec3(ph, 0., -de)), y, dot(vec3(de, 0., ph)));
        float lbd = 1. / sqrtf(1. - naxis.y);
        vec3 abc = ((up - naxis * naxis.y) * lbd).normalize();
        vec3 cr = abc.cross(naxis);

        // change of basis then rotation around (Oy) then re-change of basis (product of 3 matrices)
        vec3 L1((abc.x * ph - cr.x * de) * abc.x + naxis.x * naxis.x + (abc.x * de + cr.x * ph) * cr.x, (abc.y * ph - cr.y * de) * abc.x + naxis.x * naxis.y + (abc.y * de + cr.y * ph) * cr.x, (abc.z * ph - cr.z * de) * abc.x + naxis.x * naxis.z + (abc.z * de + cr.z * ph) * cr.x),
             L2((abc.x * ph - cr.x * de) * abc.y + naxis.y * naxis.x + (abc.x * de + cr.x * ph) * cr.y, (abc.y * ph - cr.y * de) * abc.y + naxis.y * naxis.y + (abc.y * de + cr.y * ph) * cr.y, (abc.z * ph - cr.z * de) * abc.y + naxis.y * naxis.z + (abc.z * de + cr.z * ph) * cr.y),
             L3((abc.x * ph - cr.x * de) * abc.z + naxis.z * naxis.x + (abc.x * de + cr.x * ph) * cr.z, (abc.y * ph - cr.y * de) * abc.z + naxis.z * naxis.y + (abc.y * de + cr.y * ph) * cr.z, (abc.z * ph - cr.z * de) * abc.z + naxis.z * naxis.z + (abc.z * de + cr.z * ph) * cr.z);
        
        return vec3(dot(L1.normalize()), dot(L2.normalize()), dot(L3.normalize())); // matrix multiplication
    }
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

struct mat3 {
    vec3 C1, C2, C3;

    mat3() : C1(vec3(1.,0.,0.)), C2(vec3(0.,1.,0.)), C3(vec3(0.,0.,1.)) {}
    mat3(vec3 m[3]) { C1 = m[0]; C2 = m[1]; C3 = m[2]; }
    mat3(const vec3& c1, const vec3& c2, const vec3& c3) : C1(c1), C2(c2), C3(c3) {}

    mat3 transpose() const { return mat3(vec3(C1.x, C2.x, C3.x), vec3(C1.y, C2.y, C3.y), vec3(C1.z, C2.z, C3.z)); }
    vec3 operator*(const vec3& v) const { return vec3(v.dot(vec3(C1.x, C2.x, C3.x)), v.dot(vec3(C1.y, C2.y, C3.y)), v.dot(vec3(C1.z, C2.z, C3.z))); }
    mat3 operator*(const mat3& o) const
    {
        mat3 T = transpose();
        return mat3(vec3(T.C1.dot(o.C1), T.C1.dot(o.C2), T.C1.dot(o.C3)), 
                    vec3(T.C2.dot(o.C1), T.C2.dot(o.C2), T.C2.dot(o.C3)), 
                    vec3(T.C3.dot(o.C1), T.C3.dot(o.C2), T.C3.dot(o.C3)));
    }

    void coefs(float* p) const { if(!p) return;
        p[0] =C1.x;    p[3] = C2.x;   p[6] = C3.x; 
        p[1] =C1.y;    p[4] = C2.y;   p[7] = C3.y;
        p[2] =C1.z;    p[5] = C2.z;   p[8] = C3.z;
    }
};


#endif // MATH_H