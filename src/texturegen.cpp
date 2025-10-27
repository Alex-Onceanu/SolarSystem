
#include "../include/math.hpp"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

float atmosFalloff = 4.;
float atmosRadius = 14.0;
float planetRadius = 60.;

float nb_steps = 50.;

vec2 raySphere(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius)
{
    vec3 p = rayPos - sphPos;
    float delta = 4. * (vec3::dot(p, rayDir) * vec3::dot(p, rayDir) - vec3::dot(rayDir, rayDir) * (vec3::dot(p, p) - radius * radius));
    if(delta < 0.) return vec2(1e5, -1e5);
    return vec2((-2. * vec3::dot(p, rayDir) - sqrt(delta)) / (2. * vec3::dot(rayDir, rayDir)), 
                (-2. * vec3::dot(p, rayDir) + sqrt(delta)) / (2. * vec3::dot(rayDir, rayDir)));
}

float densityAtPoint(float h)
{
    return exp(-h * atmosFalloff) * (1. - h);
}

float opticalDepth(float h, float cosTheta)
{
    vec3 start(0., planetRadius + h * atmosRadius, 0.);
    vec3 rayDir(sqrtf(1. - cosTheta * cosTheta), cosTheta, 0.);
    float rayLength = raySphere(start, rayDir, vec3(0., 0., 0.), planetRadius + atmosRadius).y;

    float dt = rayLength / nb_steps;
    vec3 p = start;
    float opticalDepth = 0.;
    for(float t = dt; t < rayLength; t += dt)
    {
        p = start + rayDir * t;
        opticalDepth += dt * densityAtPoint(h);
    }

    return opticalDepth;
}


// I tried to use this to bake the optical depth instead of having an inner loop in the atmosphere shader
// but it didn't look great and also I only gained 0.002 ms of performance which is negligeable...
// so I abandoned the idea of precomputing the atmosphere and am still doing it in real-time with riemann sum
void generateOpticalDepthTexture()
{
    constexpr size_t RESOLUTION = 1024;
    char img[RESOLUTION * RESOLUTION];

    float maxfound = 0.;

    for(int i = 0; i < RESOLUTION; i++)
    {
        for(int j = 0; j < RESOLUTION; j++)
        {
            float h = static_cast<float>(j + 1) / static_cast<float>(RESOLUTION);
            float cosTheta = 2.0 * static_cast<float>(i + 1) / static_cast<float>(RESOLUTION) - 1.;
            float od = opticalDepth(h, cosTheta);
            maxfound = std::max(maxfound, od);

            img[i * RESOLUTION + j] = static_cast<char>(od * 255. / 134.);
        }
    }

    std::cout << "max found : " << maxfound << std::endl;

    std::ofstream out("output.pgm");
    std::string sres = std::to_string(RESOLUTION);
    out << "P5\n" << sres << " " << sres << "\n" << "255\n";
    out.write(img, RESOLUTION * RESOLUTION);
}

float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - vec4::floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + vec4(1.,1.,1.,1.)) * x);}

float noise(vec3 p) {
    vec3 a = vec3::floor(p);
    vec3 d = p - a;
    d = d * d * (vec3(3.,3.,3.) - d * 2.);

    vec4 b = vec4(a.x, a.x, a.y, a.y) + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(vec4(b.x, b.y, b.x, b.y));
    vec4 k2 = perm(vec4(k1.x, k1.y, k1.x, k1.y) + vec4(b.z, b.z, b.w, b.w));

    vec4 c = k2 + vec4(a.z, a.z, a.z, a.z);
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + vec4(1.,1.,1.,1.));

    vec4 o1 = vec4::fract(k3 * (1.0 / 41.0));
    vec4 o2 = vec4::fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = vec2(o3.y, o3.w) * d.x + vec2(o3.x, o3.z) * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

// float noise(vec3 p)
// {
//     return 0.;
// }

// sum of perlin noise for yellow noise (fractional brownian motion) : see https://iquilezles.org/articles/fbm/
float fbm(vec3 x)
{    
    float G = 1.0 / 2.71828;
    float f = 8.0;
    float a = 1.0;
    float t = 0.0;
    int numOctaves = 6;
    for( int i=0; i<numOctaves; i++ )
    {
        t += noise(x * f) * a;
        f *= 2.0;
        a *= G;
    }
    return t;
}

void generateSphericalFBMnoise()
{
    constexpr size_t RESOLUTION = 4096 * 4;
    char* img = new char[RESOLUTION * RESOLUTION];

    float maxfound = 0.;
    float minfound = INFINITY;

    for(int i = 0; i < RESOLUTION; i++)
    {
        for(int j = 0; j < RESOLUTION; j++)
        {
            vec2 uv(static_cast<float>(j) / static_cast<float>(RESOLUTION), static_cast<float>(i) / static_cast<float>(RESOLUTION));
            vec3 d(cosf(M_PI * (0.5 - uv.y)) * cosf(2. * M_PI * (uv.x - 0.5)), sinf(M_PI * (0.5 - uv.y)), cosf(M_PI * (0.5 - uv.y)) * sinf(2. * M_PI * (uv.x - 0.5)));
            d.normalized();
            float x = fbm(d);
            minfound = std::min(minfound, x);
            maxfound = std::max(maxfound, x);

            img[i * RESOLUTION + j] = static_cast<char>(CLAMP((x - minfound) / (maxfound - minfound), 0.f, 1.f) * 255.);
        }
        std::cout << i << std::endl;
    }

    std::cout << "min : " << minfound << ", max : " << maxfound << std::endl;

    std::ofstream out("output.pgm");
    std::string sres = std::to_string(RESOLUTION);
    out << "P5\n" << sres << " " << sres << "\n" << "255\n";
    out.write(img, RESOLUTION * RESOLUTION);
    delete[] img;
}

// returns 3D value noise and its 3 derivatives
// vec4 noised(vec3 x)
// {
//     vec3 p = vec3::floor(x);
//     vec3 w = vec3::fract(x);

//     vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
//     vec3 du = 30.0*w*w*(w*(w-1.0)+2.0);

//     float a = myRandomMagic( p+vec3(0,0,0) );
//     float b = myRandomMagic( p+vec3(1,0,0) );
//     float c = myRandomMagic( p+vec3(0,1,0) );
//     float d = myRandomMagic( p+vec3(1,1,0) );
//     float e = myRandomMagic( p+vec3(0,0,1) );
//     float f = myRandomMagic( p+vec3(1,0,1) );
//     float g = myRandomMagic( p+vec3(0,1,1) );
//     float h = myRandomMagic( p+vec3(1,1,1) );

//     float k0 =   a;
//     float k1 =   b - a;
//     float k2 =   c - a;
//     float k3 =   e - a;
//     float k4 =   a - b - c + d;
//     float k5 =   a - c - e + g;
//     float k6 =   a - b - e + f;
//     float k7 = - a + b + c - d + e - f - g + h;

//     return vec4( -1.0+2.0*(k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z),
//                  2.0*du *vec3( k1 + k4*u.y + k6*u.z + k7*u.y*u.z,
//                                k2 + k5*u.z + k4*u.x + k7*u.z*u.x,
//                                k3 + k6*u.x + k5*u.y + k7*u.x*u.y ) );
// }

// // returns 3D fbm and its 3 derivatives
// vec4 fbmNormal(vec3 x)
// {
//     float f = 1.98;  // could be 2.0
//     float s = 0.49;  // could be 0.5
//     float a = 0.0;
//     float b = 0.5;
//     int numOctaves = 6;
//     vec3  d = vec3(0.0,0.0,0.0);
//     mat3  m = mat3(1.0,0.0,0.0,
//                    0.0,1.0,0.0,
//                    0.0,0.0,1.0);
//     for( int i=0; i<numOctaves; i++ )
//     {
//         vec4 n = noised(x);
//         a += b*n.x;          // accumulate values
//         d += b*m*n.yzw;      // accumulate derivatives
//         b *= s;
//         x = f*m3*x;
//         m = f*m3i*m;
//     }
//     return vec4(a, d.x, d.y, d.z);
// }

// void generateSphericalFBMnormalMap()
// {
//     constexpr size_t RESOLUTION = 4096;
//     char* img = new char[RESOLUTION * RESOLUTION];

//     float maxfound = 0.;
//     float minfound = INFINITY;

//     for(int i = 0; i < RESOLUTION; i++)
//     {
//         for(int j = 0; j < RESOLUTION; j++)
//         {
//             vec2 uv(static_cast<float>(j) / static_cast<float>(RESOLUTION), static_cast<float>(i) / static_cast<float>(RESOLUTION));
//             vec3 d(cosf(M_PI * (0.5 - uv.y)) * cosf(2. * M_PI * (uv.x - 0.5)), sinf(M_PI * (0.5 - uv.y)), cosf(M_PI * (0.5 - uv.y)) * sinf(2. * M_PI * (uv.x - 0.5)));
//             d.normalized();
//             vec3 n = fbmNormal(d);

//             img[3 * (i * RESOLUTION + j)] = static_cast<char>(0.5 * (n.x + 1.) * 255.f);
//             img[3 * (i * RESOLUTION + j + 1)] = static_cast<char>(0.5 * (n.y + 1.) * 255.f);
//             img[3 * (i * RESOLUTION + j + 2)] = static_cast<char>(0.5 * (n.z + 1.) * 255.f);
//         }
//         std::cout << i << std::endl;
//     }

//     std::ofstream out("output.ppm");
//     std::string sres = std::to_string(RESOLUTION);
//     out << "P6\n" << sres << " " << sres << "\n" << "255\n";
//     out.write(img, RESOLUTION * RESOLUTION);
//     delete[] img;
// }

int main()
{
    srand(time(NULL));
    generateSphericalFBMnoise();
    return 0;
}