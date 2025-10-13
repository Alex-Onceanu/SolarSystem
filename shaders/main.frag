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

uniform float NB_STEPS_i;
uniform float NB_STEPS_j;
uniform float atmosFalloff;
uniform float atmosRadius;
uniform vec3 atmosColor;

uniform float mountainAmplitude;
uniform float mountainFrequency;

mat2 rot2D(float theta)
{
    return mat2(vec2(cos(theta), -sin(theta)), vec2(sin(theta), cos(theta)));
}


float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

float fbm(vec2 x)
{    
    float G = 1.0 / 2.71828; // H = 1 for yellow noise
    float f = 1.0;
    float a = 1.0;
    float t = 0.0;
    int numOctaves = 4;
    for( int i=0; i<numOctaves; i++ )
    {
        t += a*noise(f*x);
        f *= 2.0;
        a *= G;
    }
    return t;
}

// wikipedia.org
vec2 cubeMapUV(vec3 p)
{
    float x = p.x, y = p.y, z = p.z;

    float absX = abs(x);
    float absY = abs(y);
    float absZ = abs(z);

    int isXPositive = x > 0 ? 1 : 0;
    int isYPositive = y > 0 ? 1 : 0;
    int isZPositive = z > 0 ? 1 : 0;

    float maxAxis, uc, vc;

    // Positive X
    if (isXPositive == 1 && absX >= absY && absX >= absZ) {
    // u (0 to 1) goes from +z to −z
    // v (0 to 1) goes from −y to +y
    maxAxis = absX;
    uc = -z;
    vc = y;
    }
    // Negative X
    if (isXPositive == 0 && absX >= absY && absX >= absZ) {
    // u (0 to 1) goes from −z to +z
    // v (0 to 1) goes from −y to +y
    maxAxis = absX;
    uc = z;
    vc = y;
    }
    // Positive Y
    if (isYPositive == 1 && absY >= absX && absY >= absZ) {
    // u (0 to 1) goes from −x to +x
    // v (0 to 1) goes from +z to −z
    maxAxis = absY;
    uc = x;
    vc = -z;
    }
    // Negative Y
    if (isYPositive == 0 && absY >= absX && absY >= absZ) {
    // u (0 to 1) goes from −x to +x
    // v (0 to 1) goes from −z to +z
    maxAxis = absY;
    uc = x;
    vc = z;
    }
    // Positive Z
    if (isZPositive == 1 && absZ >= absX && absZ >= absY) {
    // u (0 to 1) goes from −x to +x
    // v (0 to 1) goes from −y to +y
    maxAxis = absZ;
    uc = x;
    vc = y;
    }
    // Negative Z
    if (isZPositive == 0 && absZ >= absX && absZ >= absY) {
    // u (0 to 1) goes from +x to −x
    // v (0 to 1) goes from −y to +y
    maxAxis = absZ;
    uc = -x;
    vc = y;
    }

    // Convert range from −1 to 1 to 0 to 1
    return vec2(0.5f * (uc / maxAxis + 1.0f), 0.5f * (vc / maxAxis + 1.0f));
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
        // vec2 uv = vec2(0.5 + atan(d.z, d.x) / (2. * 3.1416), 0.5 - asin(d.y) / 3.1416);
        vec2 uv = cubeMapUV(d);
        float h = mountainAmplitude * fbm(mountainFrequency * uv);

        foundMountain = foundMountain || (ph < h);
        t += dt * goingDown * (ph >= h ? 1. : -1.);
        dt /= 2.;
    }
    if(!foundMountain) return -1.; // TODO : return 1e5 et se debarrasser du bool
    return t;
}

vec4 rayCastMountains(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius, float tPlanety)
{
    const int NB_ITERATIONS = 16;
    const int HALF_NB_ITERATIONS = 8;
    float goingDown = 1.;
    float tmax = 1.;
    vec2 planetHitInfo = raySphere(rayPos, rayDir, sphPos, radius);
    float t = 0.5;
    if(planetHitInfo.x > planetHitInfo.y || planetHitInfo.y < 0.)
    {
        tmax = raySphereMinDist(rayPos, rayDir, sphPos, radius).y;
        t = binarySearchMountain(rayPos, rayDir, sphPos, radius, tmax, 1., NB_ITERATIONS);
        if(t < 0.) // if the ray didn't find any mountain while going down to min, search when ascending
            t = binarySearchMountain(rayPos + tmax * rayDir, rayDir, sphPos, radius, tPlanety - tmax, -1., NB_ITERATIONS);
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

    vec4 mtn = rayCastMountains(pos, rayDir, spherePos, radius, tPlanety);
    float n = mtn.w / mountainAmplitude;
    if(n < 0.) return vec3(-1.);
    vec3 clr;

    if(n < 0.4) clr = vec3(79., 76., 176.) / 255.;
    else if(n < 0.45) clr = vec3(216., 197., 150.) / 255.;
    else if(n < 0.65) clr = vec3(159., 193., 100.) / 255.;
    else clr = vec3(195.,146.,79.) / 255.;

    return max(0.2, dot(normalize(pos - spherePos), normalize(lightSource - pos))) * clr;
}


vec3 background(vec3 rayDir)
{
    vec3 d = normalize(rayDir);
    vec2 uv = vec2(0.5 + atan(d.z, d.x) / (2. * 3.1416), 0.5 - asin(d.y) / 3.1416);
    return vec3(smoothstep(0.96, 0.98, noise(1000. * uv)));
}

float densityAtPoint(vec3 where, vec3 planetPos, float planetRadius)
{
    float h = length(where - planetPos) - planetRadius;
    float h01 = h / atmosRadius;
    return exp(-h01 * atmosFalloff) * (1. - h01);
}

float opticalDepthFast(vec3 rayDir, vec3 rayPos, float rayLength, float nb_steps, vec3 planetPos, float planetRadius)
{
    const float r = 0.86, q = 1.25; // magic numbers :)
    float m = (r * r) / (1. - r * r);
    float mp = 1. - 1. / r;
    const float e = 2.71828;
    float qp = (e * q) / (q + e - q * e) - q;
    float qpp = 1. - 1. / q;
    float rdl2 = dot(rayDir, rayDir);

    vec3 j = rayPos - planetPos;
    float dotrdj = dot(rayDir, j);
    float jl2 = dot(j, j);

    float ar2 = atmosRadius * atmosRadius;
    float K = exp(planetRadius / (atmosRadius * atmosFalloff));
    float Kp = planetRadius * K / atmosRadius;

    float A = rdl2 * m / ar2;
    float B = (2. * m * dotrdj) / ar2;
    float C = r + (m * jl2) / ar2;
    float delta = sqrt(4. * A * C - B * B);

    float D = rdl2 * qp / ar2;
    float E = 2. * qp * dotrdj / ar2;
    float F = q + qp * jl2 / ar2;
    float deltap = sqrt(4. * D * F - E * E);

    float integral1 = atan(2. * A * rayLength + B, delta) - atan(B, delta);
    float integral2 = atan(2. * D * rayLength + E, deltap) - atan(F, deltap);

    float rem = (K * mp - Kp * qpp) * rayLength;

    return (2. * K / delta) * integral1 - (2. * Kp / deltap) * integral2 + rem;
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

        float iOpticalDepth = opticalDepth(toLight, p, rayLengthToSky, NB_STEPS_j, planetPos, 1.);
        toEyeOpticalDepth = opticalDepth(-rayDir, p, t, NB_STEPS_j, planetPos, 1.);
        vec3 transmittance = exp(-(iOpticalDepth + toEyeOpticalDepth) * atmosColor);
        float localDensity = densityAtPoint(p, planetPos, radius);

        totalLight += localDensity * transmittance * atmosColor * idt;
    }

    return totalLight + exp(-toEyeOpticalDepth) * originalColor;
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
            vec3 mountainColor = shadePlanet(rayDir, rayPos + max(0., tPlanet.x) * rayDir, 
                            planets[i], planetRadius, sunPos, tPlanet.y - max(0., tPlanet.x));
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