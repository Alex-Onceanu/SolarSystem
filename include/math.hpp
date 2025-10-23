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

class vec3 {
public:
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
    float dot(const vec3& other) const { return x * other.x + y * other.y; }
    vec3 cross(const vec3& o) { return vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x); }
    float length() { return sqrtf(x * x + y * y + z * z); }
    void normalized() { float l = length(); if(l <= 0.) return; x /= l; y /= l; z /= l; }
    vec3 normalize() { vec3 ans = *this; ans.normalized(); return ans; }

    friend std::ostream& operator<<(std::ostream& stream, const vec3& v)
    {
        stream << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return stream;
    }

    static float dot(const vec3& x, const vec3& y) { return x.dot(y); }
    static vec3 floor(const vec3& p) { return vec3((float)(int)p.x - (p.x < 0. ? 1. : 0.), (float)(int)p.y - (p.y < 0. ? 1. : 0.), (float)(int)p.z - (p.z < 0. ? 1. : 0.)); }

private:

    struct mat4_t
    {
        float coefs[16];
        float operator[](size_t i) { return coefs[i]; }
    };

    mat4_t mat4_id_t()
    {
        mat4_t res;

        for (int i = 0; i < 16; ++i)
        {
            res.coefs[i] = (int)(i % 4 == i / 4);
        }

        return res;
    }

    mat4_t mat4_produit(const mat4_t m1, const mat4_t m2)
    {
        mat4_t res;

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                res.coefs[4 * i + j] = 0.0;
                for (int k = 0; k < 4; ++k)
                {
                    res.coefs[4 * i + j] += m1.coefs[4 * i + k] * m2.coefs[k * 4 + j];
                }
            }
        }

        return res;
    }

    mat4_t mat4_transpose(const mat4_t m)
    {
        mat4_t res = mat4_id_t();
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                res.coefs[4 * i + j] = m.coefs[4 * j + i];
            }
        }
        return res;
    }

    mat4_t rotation_x_4(float theta)
    {
        mat4_t res = mat4_id_t();

        res.coefs[5] = cos(theta);
        res.coefs[6] = -sin(theta);
        res.coefs[9] = sin(theta);
        res.coefs[10] = cos(theta);

        return res;
    }

    mat4_t rotation_y_4(float theta)
    {
        mat4_t res = mat4_id_t();

        res.coefs[0] = cos(theta);
        res.coefs[2] = sin(theta);
        res.coefs[8] = -sin(theta);
        res.coefs[10] = cos(theta);

        return res;
    }

    mat4_t rotation_z_4(float theta)
    {
        mat4_t res = mat4_id_t();

        res.coefs[0] = cos(theta);
        res.coefs[1] = -sin(theta);
        res.coefs[4] = sin(theta);
        res.coefs[5] = cos(theta);

        return res;
    }

    mat4_t mat4_rotation(vec3 axe, bool inverse, mat4_t* passage)
    {
        float angle = axe.length() * (inverse ? -1. : 1.);
        // std::cout << angle << std::endl;

        if(fabsf(angle) <= 0.000001)
        {
            if(passage != NULL) *passage = mat4_id_t();
            return mat4_id_t();
        }

        vec3 theta = axe;
        theta.normalized();
        // std::cout << theta << std::endl;

        mat4_t P = mat4_id_t();

        // Matrice de passage vers une nouvelle base dans laquelle theta est le nouvel axe Ox
        float d = sqrtf(theta.x * theta.x + theta.y * theta.y);

        if(d <= 0.000001)
        {
            // Si on entre dans ce if la rotation est nécessairement autour de Oz
            // La matrice de passage est alors celle qui place vec{e_x} sur + ou - vec{e_z}
            if(passage != NULL) *passage = rotation_y_4(- (theta.z / fabsf(theta.z)) * M_PI_2);
            return rotation_z_4(angle);
        }

        P.coefs[0] = theta.x;
        P.coefs[1] = theta.y;
        P.coefs[2] = theta.z;
        P.coefs[4] = -theta.y / d;
        P.coefs[5] = theta.x / d;
        P.coefs[6] = 0.0;
        P.coefs[8] = -theta.z * P.coefs[5];
        P.coefs[9] = theta.z * P.coefs[4];
        P.coefs[10] = theta.x * P.coefs[5] - theta.y * P.coefs[4];

        mat4_t P_T = mat4_transpose(P);

        if(passage != NULL) *passage = P_T;

        return mat4_produit(P_T, mat4_produit(rotation_x_4(angle), P));
    }
public:
    vec3 rotate(vec3 axis, bool inverse=false, mat4_t* P=nullptr)
    { 
        mat4_t R = mat4_rotation(axis, inverse, P); 
        return vec3(dot(vec3(R[0], R[1], R[2])), dot(vec3(R[4], R[5], R[6])), dot(vec3(R[8], R[9], R[10])));
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




#endif // MATH_H