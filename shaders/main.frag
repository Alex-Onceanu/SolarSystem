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

uniform float ambientCoef;
uniform float diffuseCoef;
uniform float minDiffuse;
uniform float penumbraCoef;

uniform sampler2D earthTexture;
uniform sampler2D opticalDepthTexture;

uniform float NB_STEPS_i;
uniform float NB_STEPS_j;
uniform float atmosFalloff;
uniform float atmosRadius;
uniform vec3 atmosColor;

uniform float mountainAmplitude;
uniform float mountainFrequency;

uniform float seaLevel;
uniform vec4 waterColor;
uniform float refractionindex;
uniform float fresnel;

uniform float nbStars;
uniform float starsDisplacement;
uniform float starSize;
uniform float starSizeVariation;
uniform float starVoidThreshold;
uniform float starFlickering;



// _____________________________________________ UTILITY FUNCTIONS _____________________________________________________

mat2 rot2D(float theta)
{
    return mat2(vec2(cos(theta), -sin(theta)), vec2(sin(theta), cos(theta)));
}

// see texturegen.cpp for yellow noise generation
float noise(vec3 d, bool underwater)
{
    vec2 uv = vec2(0.5 + atan(d.z, d.x) / (2. * 3.1416), 0.5 - asin(d.y) / 3.1416);
    float x = texture(opticalDepthTexture, uv).r;
    return max(x, underwater ? 0. : seaLevel);
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
    const float kNum = nbStars;

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

// _____________________________________________________ BACKGROUND ______________________________________________________

vec2 raySphereMinDist(vec3 rayPos, vec3 rayDir, vec3 spherePos, float radius)
{
    float t = -dot(rayDir, rayPos - spherePos) / dot(rayDir, rayDir);
    if(t <= 0.) return vec2(1e5, t);
    vec3 pos = rayPos + t * rayDir;
    return vec2(length(pos - spherePos) - radius, t);
}

vec3 background(vec3 d)
{
    vec3 nd = normalize(d);
    vec2 centered = inverseSF(nd);
    float seed = centered.x;

    float rand1 = rand(seed);
    float rand2 = rand(rand1);
    float rand3 = rand(rand2);
    vec3 randVector = vec3(rand1, rand2, rand3);
    
    if(rand1 < starVoidThreshold) return vec3(0.); // so we have some void

    // second call to inverseSF because we needed to get the seed first
    // now we can use the seed to offset the stars for a more "natural" look
    vec2 a = inverseSF(normalize(nd + starsDisplacement * (-1. + 2. * randVector)));

    float dst = (starSize + starSizeVariation * rand1 + starFlickering * rand2 * pow(sin(3. * time * rand3), 5.)) * a.y;

    float glow = 1. / (0.001 + dst * dst);
    vec3 clr = 1. + 0.4 * randVector;
    float border = 1. - smoothstep(0.0, 0.015, centered.y); // temporary fix to the "neighbours" issue

    return tanh(glow * clr) * border;
}

// _____________________________________________________ PLANET ________________________________________________________

vec4 rayCastMountains(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius, float tPlanety, bool underwater)
{
    float nb_iterations = underwater ? 40. : 300.;
    float maxt = tPlanety;
    float dt = max(0.05, maxt / nb_iterations);
    float lh = 0.0;
    float ly = 0.0;

    for(float t = 0.001; t < maxt; t += dt)
    {
        vec3 p = rayPos + t * rayDir;
        float py = length(p - sphPos) - radius;
        vec3 d = normalize(p - sphPos);
        // d.xz *= rot2D(0.1 * time);
        float h = mountainAmplitude * noise(d, underwater);
        if(py < h)
        {
            float dst = t-dt+dt*(lh-ly)/(py-ly-h+lh); // <--- https://iquilezles.org/articles/terrainmarching/
            return vec4(rayPos + dst * rayDir, h);
        }
        lh = h;
        ly = py;
    }
    return vec4(-1.);
}

// .w of return value is positive if there is a reflection
vec4 shadePlanet(vec3 rayDir, vec3 pos, vec3 spherePos, float radius, vec3 lightSource, float tPlanety)
{
    vec4 mtn = rayCastMountains(pos, rayDir, spherePos, radius, tPlanety, false);
    float n = mtn.w / mountainAmplitude;

    if(n < 0.) return vec4(-1.);
    vec3 clr;

    float shouldReflect = -1.;

    vec3 sphereNormal = normalize(mtn.xyz - spherePos);
    // sphereDiffuse doesn't account for mountains
    float sphereDiffuse = max(minDiffuse, dot(normalize(mtn.xyz - spherePos), normalize(lightSource - mtn.xyz)));

    if(n <= seaLevel + 0.0001)
    {
        // refract
        clr = waterColor.rgb;

        vec3 refracted = refract(rayDir, sphereNormal, refractionindex);
        vec2 dstToSeabed = raySphere(mtn.xyz, refracted, spherePos, radius);
        float refrCoef = abs(dot(normalize(refracted), sphereNormal));
        if(dstToSeabed.x > dstToSeabed.y)
        {
            refrCoef = 0.; // perfect reflection if the refracted vector doesn't find the seabed (not physically based but looks pretty)
        }
        else
        {
            mtn = rayCastMountains(mtn.xyz, refracted, spherePos, radius, dstToSeabed.x, true);
            clr = mix(clr, vec3(195.,146.,79.) / 255., waterColor.a);
        }
        shouldReflect = 1. - (fresnel + refrCoef * (1. - fresnel));
    }
    else if(n < seaLevel + 0.05) clr = vec3(216., 197., 150.) / 255.;
    else if(n < 0.6) clr = vec3(195.,146.,79.) / 255.;
    else clr = vec3(159., 193., 100.) / 255.;

    vec2 eps = vec2(0.005, 0.);

    // derivative of implicit surface is (dF/dx, dF/dy, dF/dz) so here (-df/dx, 1, -df/dz)
    vec3 sample1 = mtn.xyz + eps.xyy;
    float h1 = noise(normalize(sample1 - spherePos), n <= seaLevel + 0.0001);
    vec3 sample1b = mtn.xyz - eps.xyy;
    float h1b = noise(normalize(sample1b - spherePos), n <= seaLevel + 0.0001);
    float gradx = (h1 - h1b) / (2. * eps.x);

    vec3 sample2 = mtn.xyz + eps.yyx;
    float h2 = noise(normalize(sample2 - spherePos), n <= seaLevel + 0.0001);
    vec3 sample2b = mtn.xyz - eps.yyx;
    float h2b = noise(normalize(sample2b - spherePos), n <= seaLevel + 0.0001);
    float gradz = (h2 - h2b) / (2. * eps.x);

    // TODO : this only works when the normal is aligned with (Oy), needs to be rotated
    vec3 localNormal = normalize(vec3(-mountainAmplitude * gradx, 1., -mountainAmplitude * gradz));

    // TODO : no grass grows on slope
    // float grassOnSlope = 0.5;
    // if(abs(dot(localNormal, sphereNormal)) < grassOnSlope) 
    //     clr = vec3(195.,146.,79.) / 255.;

    // clr = mix(clr, vec3(0.34, 0.34, 0.34), 1. - smoothstep(0.0, 0.8, abs(dot(localNormal, sphereNormal))));

    float diffuse = max(0.0, dot(localNormal, normalize(lightSource - mtn.xyz)));
    float penumbra = smoothstep(0.1, 0.6, n); // trick for faking global illumination

    float light = diffuseCoef * diffuse * sphereDiffuse + penumbraCoef * penumbra;
    vec3 shaded = light * clr + ambientCoef * normalize(vec3(1.) + atmosColor);

    return vec4((shouldReflect < -0.1 ? 1. : (1. - shouldReflect)) * shaded, shouldReflect);
}

// _____________________________________________________ ATMOSPHERE ________________________________________________________

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
        toEyeOpticalDepth = opticalDepth(-rayDir, p, t, NB_STEPS_j, planetPos, radius);
        vec3 transmittance = exp(-(iOpticalDepth + toEyeOpticalDepth) * atmosColor);
        float localDensity = densityAtPoint(p, planetPos, radius);

        totalLight += localDensity * transmittance * atmosColor * idt;
    }

    return totalLight + exp(-toEyeOpticalDepth) * originalColor;
}

// _____________________________________________________ MAIN ________________________________________________________

vec3 raytraceMap(vec3 rayDir, vec3 rayPos)
{   
    vec3 mapColor = vec3(0.);
    vec3 r0 = rayPos, rd = rayDir;
    int NB_MAX_REFLECTIONS = 2;
    float reflectionCoef = 1., nextReflectionCoef = 1.;
    // no recursivity in GLSL (so we have to use loops for reflection... until I code my own shader language (in some IGR class I hope))
    for(int r = 0; r < NB_MAX_REFLECTIONS; r++)
    {
        const int NB_PLANETS = 1;
        bool shouldStop = true;

        float tMin = 1e5;
        float tToPlanet = 1e5;
        vec3 argmin = background(rd);

        float planetRadius = 60.0;

        vec2 tPlanet = raySphere(r0, rd, planetPos, planetRadius + mountainAmplitude);
        float tstart = max(0., tPlanet.x);
        if(tPlanet.y > tPlanet.x && tstart < tMin && tPlanet.y >= 0.)
        {
            vec4 mountainColor = shadePlanet(rd, r0 + tstart * rd, 
                            planetPos, planetRadius, sunPos, tPlanet.y - tstart);
            if(mountainColor.x >= -0.1)
            {
                tMin = tstart; // ah bon ?? et si tstart == 0 ???
                tToPlanet = tstart;
                argmin = mountainColor.rgb;

                shouldStop = mountainColor.w < -0.1;
                nextReflectionCoef = mountainColor.w;
            }
        }

        vec2 corona = raySphere(r0, rd, sunPos, sunCoronaStrength + 10.);
        if(corona.y > corona.x && corona.x < tMin && corona.y >= 0.)
        {
            vec2 sun = raySphere(r0, rd, sunPos, 10.0);
            if(sun.y > sun.x && sun.x < tMin && sun.y >= 0.)
            {
                tMin = sun.x;
                argmin = 2.0 * sunColor;
            }
            else
            {
                float md = (1.0 / 10.) * raySphereMinDist(r0, rd, sunPos, 10.).x + 1.;
                float light = smoothstep(0.0, 1.0, 1.0 / (md * md));
                argmin = light * sunColor + (1.0 - light) * argmin;
            }
        }

        vec2 tAtmos = raySphere(r0, rd, planetPos, planetRadius + atmosRadius);
        float dstThroughAtmosphere = min(tAtmos.y, tToPlanet - tAtmos.x);
        if(dstThroughAtmosphere > 0.)
        {
            argmin = atmosphere(rd, r0 + tAtmos.x * rd, dstThroughAtmosphere, planetPos, planetRadius, sunPos, argmin);
        }

        mapColor += argmin * reflectionCoef;

        if(!shouldStop)
        {
            float dstToWater = raySphere(r0, rd, planetPos, planetRadius + seaLevel * mountainAmplitude).x;
            r0 = r0 + dstToWater * rd;
            rd = reflect(rd, normalize(r0 - planetPos));
            reflectionCoef = nextReflectionCoef;
        }
        else break;
    }
    return mapColor;
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