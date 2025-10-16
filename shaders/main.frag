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

float binarySearchMountain( vec3 start, vec3 rayDir, vec3 sphPos, float radius, 
                            float tmax, float goingDown, int NB_ITERATIONS)
{
    float t = tmax / 2.;
    vec3 p;
    float dt = t / 2.;
    bool foundMountain = false;
    for(int i = 0; i < NB_ITERATIONS; i++)
    {
        p = start + t * rayDir;
        float ph = length(p - sphPos) - radius;
        vec3 d = normalize(p - sphPos);
        float h = mountainAmplitude * noise(d);

        foundMountain = foundMountain || (ph < h);
        t += dt * goingDown * (ph >= h ? 1. : -1.);
        dt /= 2.;
    }
    if(!foundMountain) return -1.; // TODO : return 1e5 et se debarrasser du bool
    return t;
}

vec4 rayCastMountainsLinear(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius, float tPlanety)
{
    float nb_iterations = 800.;
    float maxt = tPlanety;
    float dt = max(0.01, maxt / nb_iterations);
    float lh = 0.0;
    float ly = 0.0;
    for(float t = 0.001; t < maxt; t += dt)
    {
        vec3 p = rayPos + t * rayDir;
        float py = length(p - sphPos) - radius;
        vec3 d = normalize(p - sphPos);
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

vec4 rayCastMountains(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius, float tPlanety)
{
    const int NB_ITERATIONS = 10;
    const int HALF_NB_ITERATIONS = 8;
    float goingDown = 1.;
    float tmax = 1.;
    vec2 planetHitInfo = raySphere(rayPos, rayDir, sphPos, radius);
    float t = 0.5;
    if(planetHitInfo.x > planetHitInfo.y || planetHitInfo.y < 0.)
    {
        return rayCastMountainsLinear(rayPos, rayDir, sphPos, radius, tPlanety);
        // tmax = raySphereMinDist(rayPos, rayDir, sphPos, radius).y;
        // t = binarySearchMountain(rayPos, rayDir, sphPos, radius, tmax, 1., HALF_NB_ITERATIONS);
        // if(t < 0.) // if the ray didn't find any mountain while going down to min, search when ascending
        //     t = binarySearchMountain(rayPos + tmax * rayDir, rayDir, sphPos, radius, tPlanety - tmax, -1., HALF_NB_ITERATIONS);
    }
    else
    {
        tmax = planetHitInfo.x;
        t = binarySearchMountain(rayPos, rayDir, sphPos, radius, planetHitInfo.x, 1., NB_ITERATIONS);
    }

    if(t < 0.)
        return vec4(-1.);

    vec3 p = rayPos + t * rayDir;
    return vec4(p, length(p - sphPos) - radius);
}

vec3 shadePlanet(vec3 rayDir, vec3 pos, vec3 spherePos, float radius, vec3 lightSource, float tPlanety)
{
    vec3 d = (pos - spherePos) / radius;
    d.xz *= rot2D(0.02 * time);
    // vec2 uv = vec2(0.5 + atan(d.z, d.x) / (2. * 3.1416), 0.5 - asin(d.y) / 3.1416);
    // vec2 uv = cubeMapUV(d);

    vec4 mtn = rayCastMountainsLinear(pos, rayDir, spherePos, radius, tPlanety);
    float n = mtn.w / mountainAmplitude;
    if(n < 0.) return vec3(-1.);
    vec3 clr;

    if(n < seaLevel + 0.0001) clr = vec3(79., 76., 176.) / 255.;
    else if(n < seaLevel + 0.1) clr = vec3(216., 197., 150.) / 255.;
    else if(n < 0.6) clr = vec3(159., 193., 100.) / 255.;
    else clr = vec3(195.,146.,79.) / 255.;

    return max(0.1, dot(normalize(pos - spherePos), normalize(lightSource - pos))) * clr;
}

vec3 background(vec3 rayDir)
{
    vec3 d = normalize(rayDir);
    return vec3(0.);
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

    // float mu = dot(rayDir, normalize(lightSource - start));
    // float phase = 3.0 * (1.0 + mu * mu) / (16.0 * 3.14159265); // less scattering when orthogonal

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