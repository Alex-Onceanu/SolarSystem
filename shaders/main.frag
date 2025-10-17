#version 450

layout(location = 0) in vec2 vFragPos;
layout(location = 0) out vec4 outColor;

uniform float time;
uniform float fov;

uniform vec3 cameraPos;
uniform vec2 cameraRotation;

uniform vec3 sunPos;
uniform vec3 sunColor;
uniform float sunCoronaStrength;
uniform float aspectRatio;

uniform vec3 planetPos;
uniform vec3 planetColor;

uniform sampler2D earthTexture;
uniform sampler2D opticalDepthTexture;

uniform float NB_STEPS_i;
uniform float NB_STEPS_j;
uniform float atmosFalloff;
uniform float atmosRadius;
uniform vec3 atmosColor;

uniform float mountainAmplitude;
uniform float mountainFrequency;

float seaLevel = 0.3;

mat2 rot2D(float theta)
{
    return mat2(vec2(cos(theta), -sin(theta)), vec2(sin(theta), cos(theta)));
}

// found some nice random values at https://www.shadertoy.com/view/Xt23Ry
float rand(float co) { return fract(sin(co*(91.3458)) * 47453.5453); }
float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }
float rand(vec3 co){ return rand(co.xy+rand(co.z)); }

// copy-pasted this, generates random points on the surface of a sphere
// iq's version of Keinert et al's inverse Spherical Fibonacci Mapping code
// https://www.shadertoy.com/view/lllXz4
vec2 inverseSF( vec3 p ) 
{
    const float kTau = 6.28318530718;
    const float kPhi = (1.0+sqrt(5.0))/2.0;
    const float kNum = 10000.0;

    float k  = max(2.0, floor(log2(kNum*kTau*0.5*sqrt(5.0)*(1.0-p.z*p.z))/log2(kPhi+1.0)));
    float Fk = pow(kPhi, k)/sqrt(5.0);
    vec2  F  = vec2(round(Fk), round(Fk*kPhi)); // |Fk|, |Fk+1|
    
    vec2  ka = 2.0*F/kNum;
    vec2  kb = kTau*(fract((F+1.0)*kPhi)-(kPhi-1.0));    

    mat2 iB = mat2( ka.y, -ka.x, kb.y, -kb.x ) / (ka.y*kb.x - ka.x*kb.y);
    vec2 c = floor(iB*vec2(atan(p.y,p.x),p.z-1.0+1.0/kNum));

    float d = 8.0;
    float j = 0.0;
    for( int s=0; s<4; s++ ) 
    {
        vec2  uv = vec2(s&1,s>>1);
        float id = clamp(dot(F, uv+c),0.0,kNum-1.0); // all quantities are integers
        
        float phi      = kTau*fract(id*kPhi);
        float cosTheta = 1.0 - (2.0*id+1.0)/kNum;
        float sinTheta = sqrt(1.0-cosTheta*cosTheta);
        
        vec3 q = vec3( cos(phi)*sinTheta, sin(phi)*sinTheta, cosTheta );
        float tmp = dot(q-p, q-p);
        if( tmp<d ) 
        {
            d = tmp;
            j = id;
        }
    }
    return vec2( j, sqrt(d) );
}

// see texturegen.cpp for yellow noise generation
float noise(vec3 d)
{
    vec2 uv = vec2(0.5 + atan(d.z, d.x) / (2. * 3.1416), 0.5 - asin(d.y) / 3.1416);
    float x = texture(opticalDepthTexture, uv).r;
    return max(x, seaLevel);
}

// Returns .x > .y if no intersection
vec2 raySphere(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius)
{
    vec3 p = rayPos - sphPos;
    float delta = 4. * (dot(p, rayDir) * dot(p, rayDir) - dot(rayDir, rayDir) * (dot(p, p) - radius * radius));
    if(delta < 0.) return vec2(1e5, -1e5);
    return vec2((-2. * dot(p, rayDir) - sqrt(delta)) / (2. * dot(rayDir, rayDir)), 
                (-2. * dot(p, rayDir) + sqrt(delta)) / (2. * dot(rayDir, rayDir)));
}

vec2 raySphereMinDist(vec3 rayPos, vec3 rayDir, vec3 spherePos, float radius)
{
    float t = -dot(rayDir, rayPos - spherePos) / dot(rayDir, rayDir);
    if(t <= 0.) return vec2(1e5, t);
    vec3 pos = rayPos + t * rayDir;
    return vec2(length(pos - spherePos) - radius, t);
}

vec4 rayCastMountains(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius, float tPlanety)
{
    float nb_iterations = 300.;
    float maxt = tPlanety;
    float dt = max(0.05, maxt / nb_iterations);
    float lh = 0.0;
    float ly = 0.0;

    for(float t = 0.001; t < maxt; t += dt)
    {
        vec3 p = rayPos + t * rayDir;
        float py = length(p - sphPos) - radius;
        vec3 d = normalize(p - sphPos);
        d.xz *= rot2D(0.1 * time);
        float h = mountainAmplitude * noise(d);
        if(py < h)
        {
            float dst = t-dt+dt*(lh-ly)/(py-ly-h+lh);
            return vec4(rayPos + dst * rayDir, h);
        }
        lh = h;
        ly = py;
    }
    return vec4(-1.);
}

vec3 shadePlanet(vec3 rayDir, vec3 pos, vec3 spherePos, float radius, vec3 lightSource, float tPlanety)
{
    vec4 mtn = rayCastMountains(pos, rayDir, spherePos, radius, tPlanety);
    float n = mtn.w / mountainAmplitude;
    if(n < 0.) return vec3(-1.);
    vec3 clr;

    if(n < seaLevel + 0.0001) clr = vec3(79., 76., 176.) / 255.;
    else if(n < seaLevel + 0.1) clr = vec3(216., 197., 150.) / 255.;
    else if(n < 0.6) clr = vec3(159., 193., 100.) / 255.;
    else clr = vec3(195.,146.,79.) / 255.;

    return max(0.1, dot(normalize(pos - spherePos), normalize(lightSource - pos))) * clr;
}


vec3 background(vec3 d)
{
    vec2 a = inverseSF(normalize(d));
    return (1. - smoothstep(0.003, 0.005, a.y)) * vec3(rand(a.x), rand(rand(a.x)), rand(rand(rand(a.x))));
}

float densityAtPoint(vec3 where, vec3 planetPos, float planetRadius)
{
    float h = length(where - planetPos) - planetRadius;
    return exp(-h * atmosFalloff / atmosRadius) * (1. - h / atmosRadius);
}

float opticalDepth(vec3 rayDir, vec3 rayPos, float rayLength, float nb_steps, vec3 planetPos, float planetRadius)
{
    vec3 p = rayPos;
    float dt = rayLength / nb_steps;
    float opticalDepth = 0.;

    for(float t = dt; t < rayLength; t += dt)
    {
        p = rayPos + t * rayDir;
        opticalDepth += dt * densityAtPoint(p, planetPos, planetRadius);
    }

    return opticalDepth;
}

vec3 atmosphere(vec3 rayDir, vec3 start, float dist, vec3 planetPos, float radius, vec3 lightSource, vec3 originalColor)
{
    vec3 totalLight = vec3(0.);
    float iOpticalDepth = 0.0;
    float toEyeOpticalDepth = 0.;

    float idt = dist / NB_STEPS_i;
    for(float t = idt; t <= dist; t += idt)
    {
        vec3 p = start + t * rayDir;
        vec3 toLight = normalize(lightSource - p);
        float height = length(p - planetPos) - radius;
        float rayLengthToSky = raySphere(p, toLight, planetPos, radius + atmosRadius).y;

        float iOpticalDepth = opticalDepth(toLight, p, rayLengthToSky, NB_STEPS_j, planetPos, radius);
        vec3 transmittance = exp(-iOpticalDepth * atmosColor);
        float localDensity = densityAtPoint(p, planetPos, radius);

        totalLight += localDensity * transmittance * atmosColor * idt;
    }

    return totalLight + originalColor;
}

vec3 raytraceMap(vec3 rayDir, vec3 rayPos)
{   
    const int NB_PLANETS = 1;
    vec3 planets[NB_PLANETS] = { planetPos };

    float tMin = 1e5;
    float tToPlanet = 1e5;
    vec3 argmin = background(rayDir);

    float planetRadius = 60.0;
    for(int i = 0; i < NB_PLANETS; i++)
    {
        vec2 tPlanet = raySphere(rayPos, rayDir, planets[i], planetRadius + mountainAmplitude);
        if(tPlanet.y > tPlanet.x && tPlanet.x < tMin && tPlanet.y >= 0.)
        {
            vec3 mountainColor = shadePlanet(rayDir, rayPos + tPlanet.x * rayDir, 
                            planets[i], planetRadius, sunPos, tPlanet.y - tPlanet.x);
            if(mountainColor.x >= -0.1)
            {
                tMin = tPlanet.x;
                tToPlanet = tPlanet.x;
                argmin = mountainColor;
            }
        }
    }

    vec2 corona = raySphere(rayPos, rayDir, sunPos, sunCoronaStrength + 10.);
    if(corona.y > corona.x && corona.x < tMin && corona.y >= 0.)
    {
        vec2 sun = raySphere(rayPos, rayDir, sunPos, 10.0);
        if(sun.y > sun.x && sun.x < tMin && sun.y >= 0.)
        {
            tMin = sun.x;
            argmin = 2.0 * sunColor;
        }
        else
        {
            float md = (1.0 / 10.) * raySphereMinDist(rayPos, rayDir, sunPos, 10.).x + 1.;
            float light = smoothstep(0.0, 1.0, 1.0 / (md * md));
            argmin = light * sunColor + (1.0 - light) * argmin;
        }
    }

    vec2 tAtmos = raySphere(rayPos, rayDir, planets[0], planetRadius + atmosRadius);
    float dstThroughAtmosphere = min(tAtmos.y, tToPlanet - tAtmos.x);
    if(dstThroughAtmosphere > 0.)
    {
        return atmosphere(rayDir, rayPos + tAtmos.x * rayDir, dstThroughAtmosphere, planets[0], planetRadius, sunPos, argmin);
    }

    return argmin;
}

void main()
{
    vec2 uv = vFragPos;
    uv.x *= aspectRatio;

    vec3 rayDir = vec3(uv.x, uv.y, 2. / tan(0.5 * fov));
    rayDir.yz *= rot2D(-cameraRotation.y);
    rayDir.xz *= rot2D(cameraRotation.x);

    float distToScreen = length(vec3(uv.x, uv.y, 2. / tan(0.5 * fov)));
    vec3 totalLight = raytraceMap(rayDir, cameraPos + distToScreen * rayDir);

    outColor = vec4(totalLight, 1.0);
}