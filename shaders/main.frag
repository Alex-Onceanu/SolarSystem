#version 450

layout(location = 0) in vec2 vFragPos;
layout(location = 0) out vec4 outColor;

#define NB_PLANETS 8

uniform float time;
uniform float fov;
uniform float aspectRatio;

uniform vec3 cameraPos;
uniform vec2 cameraRotation;
uniform mat3 planetBasis;

uniform vec3 sunPos;
uniform float sunRadius;
uniform vec3 sunColor;
uniform float sunCoronaStrength;

uniform vec3 planetPos[NB_PLANETS];
uniform float uPlanetRadius[NB_PLANETS];
uniform vec3 beachColor[NB_PLANETS];
uniform vec3 grassColor[NB_PLANETS];
uniform vec3 peakColor[NB_PLANETS];

uniform float ambientCoef;
uniform float diffuseCoef;
uniform float minDiffuse;
uniform float penumbraCoef;

uniform sampler2D heightmap;
uniform float mountainAmplitude[NB_PLANETS];
uniform float seaLevel[NB_PLANETS];
uniform vec4 waterColor[NB_PLANETS];
uniform float refractionindex;
uniform float fresnel;

uniform float NB_STEPS_i;
uniform float NB_STEPS_j;
uniform float atmosFalloff[NB_PLANETS];
uniform float atmosRadius[NB_PLANETS];
uniform vec3 atmosColor[NB_PLANETS];

uniform float nbStars;
uniform float starsDisplacement;
uniform float starSize;
uniform float starSizeVariation;
uniform float starVoidThreshold;
uniform float starFlickering;

uniform vec3 portalPlane1, portalPlane2;
uniform vec3 portalPos1, portalPos2;
uniform float portalSize1, portalSize2;
uniform mat3 portalBasis1, portalBasis2;

// _____________________________________________ UTILITY FUNCTIONS _____________________________________________________

mat2 rot2D(float theta)
{
    return mat2(vec2(cos(theta), -sin(theta)), vec2(sin(theta), cos(theta)));
}

// see texturegen.cpp for yellow noise generation
float noise(vec3 d, bool underwater, int i)
{
    vec2 uv = vec2(0.5 + atan(d.z, d.x) / (2. * 3.14159265), 0.5 - asin(d.y) / 3.14159265);
    float x = texture(heightmap, uv).r;
    return max(x, underwater ? 0. : seaLevel[i]);
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

// returns 1e6 if no intersection
float rayCircle(vec3 rayPos, vec3 rayDir, vec3 cPos, vec3 cPlane, float radius)
{
    if(abs(dot(rayDir, cPlane)) <= 1e-6 || radius < 0.) return 1e6;

    float t = (dot(cPos, cPlane) - dot(rayPos, cPlane)) / dot(rayDir, cPlane);
    vec3 p = rayPos + t * rayDir - cPos;
    // p.y *= 0.75;
    if(length(p) > radius) return 1e6;

    return t;
}

vec2 raySphereMinDist(vec3 rayPos, vec3 rayDir, vec3 spherePos, float radius)
{
    float t = -dot(rayDir, rayPos - spherePos) / dot(rayDir, rayDir);
    if(t <= 0.) return vec2(1e5, t);
    vec3 pos = rayPos + t * rayDir;
    return vec2(length(pos - spherePos) - radius, t);
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

vec3 background(vec3 d)
{
    vec3 nd = normalize(d);
    vec2 centered = inverseSF(nd);
    float seed = centered.x;

    float rand1 = rand(seed);
    float rand2 = rand(rand1);
    float rand3 = rand(rand2);
    vec3 randVector = vec3(rand1, rand2, rand3);
    
    // cool mario galaxy background color : vec3(0.035, 0.114, 0.392)
    if(rand1 < starVoidThreshold) return vec3(0.); // so we have some void

    // second call to inverseSF because we needed to get the seed first
    // now we can use the seed to offset the stars for a more "natural" look
    vec2 a = inverseSF(normalize(nd + starsDisplacement * (-1. + 2. * randVector)));

    float dst = (starSize + starSizeVariation * rand1 + starFlickering * rand2 * pow(sin(3. * time * rand3), 5.)) * a.y;

    float glow = 1. / (0.001 + dst * dst);
    vec3 clr = 1. + 0.6 * randVector;
    float border = 1. - smoothstep(0.0, 0.015, centered.y); // temporary fix to the "neighbours" issue

    return tanh(glow * clr) * border;
}

mat3 changeOfBasis(vec3 target, vec3 up)
{
    vec3 ntarget = normalize(target);
    if(abs(ntarget.y) > 0.999)
        return mat3(1., 0., 0., 0., 1., 0., 0., 0., 1.);
    float lbd = 1. / sqrt(1. - ntarget.y);
    vec3 abc = normalize((up - ntarget * ntarget.y) * lbd);
    vec3 cr = cross(abc, ntarget);

    return mat3(abc, ntarget, cr);
}

// _____________________________________________________ WATER ________________________________________________________

// sum of 3 propagative waves based on angle and time
float waveHeight(vec3 gwhere)
{
    float waveAmp = 0.05;
    float k0 = dot(gwhere, normalize(vec3(1.1, 0.8, 1.1)));
    float k1 = dot(gwhere, normalize(vec3(-1., 1.2, 0.9)));
    float k2 = dot(gwhere, normalize(vec3(-0.8, -0.3, -0.5)));
    float k3 = dot(gwhere, normalize(vec3(0.4, -0.6, -0.3)));
    float A0 = 0.20 * 0.5 * (1. + k0 * k0);
    float A1 = 0.16 * 0.5 * (1. + k1 * k1);
    float A2 = 0.09 * 0.5 * (1. + k2 * k2);
    float A3 = 0.05 * 0.5 * (1. + k3 * k3);   
    return  waveAmp *(A0 * (1. + sin(13. * k0 + 0.9 * time))
                    + A1 * (1. + cos(16. * k1 + 1.2 * time))
                    + A2 * (1. + sin(30. * k2 + 3.4 * time))
                    + A3 * (1. + cos(45. * k3 + 6.0 * time))); 
}

// derivative of waveHeight
vec3 waveNormal(vec3 gwhere)
{
    vec2 eps = vec2(0.005, 0.);

    vec3 sample1 = normalize(gwhere + planetBasis* eps.xyy);
    float h1 = waveHeight(sample1);
    vec3 sample1b = normalize(gwhere - planetBasis * eps.xyy);
    float h1b = waveHeight(sample1b);
    float gradx = (h1 - h1b) / (2. * eps.x);

    vec3 sample2 = normalize(gwhere + planetBasis * eps.yyx);
    float h2 = waveHeight(sample2);
    vec3 sample2b = normalize(gwhere - planetBasis * eps.yyx);
    float h2b = waveHeight(sample2b);
    float gradz = (h2 - h2b) / (2. * eps.x);

    return planetBasis * normalize(vec3(-gradx, 1., -gradz));
}

// _____________________________________________________ PLANET ________________________________________________________

vec4 rayCastMountains(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius, float tPlanety, bool underwater, float lod, int i, out float tOut)
{
    float nb_iterations = underwater ? lod / 7. : lod;
    float maxt = tPlanety;
    float dt = max(0.025, maxt / nb_iterations);
    float lh = 0.0;
    float ly = 0.0;

    for(float t = 0.001; t < maxt; t += dt)
    {
        vec3 p = rayPos + t * rayDir;
        float py = length(p - sphPos) - radius;
        vec3 d = normalize(p - sphPos);
        // d.xz *= rot2D(0.1 * time);
        float h = mountainAmplitude[i] * noise(d, underwater, i);
        if(py < h)
        {
            float dst = t-dt+dt*(lh-ly)/(py-ly-h+lh); // <--- https://iquilezles.org/articles/terrainmarching/
            tOut = t;
            return vec4(rayPos + dst * rayDir, h);
        }
        lh = h;
        ly = py;
    }
    return vec4(-1.);
}


// .w of return value is positive if there is a reflection
vec3 shadePlanet(vec3 rayDir, vec3 pos, vec3 spherePos, float radius, vec3 lightSource, float tPlanety, float lod, int i, out float refl, out float tOut)
{
    vec4 mtn = rayCastMountains(pos, rayDir, spherePos, radius, tPlanety, false, lod, i, tOut);
    float n = mtn.w / mountainAmplitude[i];

    if(n < -0.01) return vec3(-1.);
    vec3 clr;

    float shouldReflect = -1.;

    vec3 sphereNormal = normalize(mtn.xyz - spherePos);
    // sphereDiffuse doesn't account for mountains
    float sphereDiffuse = max(minDiffuse, dot(normalize(mtn.xyz - spherePos), normalize(lightSource - mtn.xyz)));

    if(n <= seaLevel[i] + 0.0001) // water
    {
        clr = waterColor[i].rgb;
        vec3 wn = normalize(waveNormal(sphereNormal));
        // clr *= max(0., dot(wn, normalize(lightSource - mtn.xyz))); // TODO : lighting water

        vec3 refracted = refract(normalize(rayDir), wn, refractionindex);
        vec2 dstToSeabed = raySphere(mtn.xyz, refracted, spherePos, radius);
        float refrCoef = abs(dot(normalize(refracted), sphereNormal));

        float tmpT = 0.;
        mtn = rayCastMountains(mtn.xyz, refracted, spherePos, radius, dstToSeabed.x, true, lod, i, tmpT);
        clr = mix(clr, vec3(195.,146.,79.) / 255., waterColor[i].a);

        shouldReflect = 1. - pow(refrCoef, fresnel);
    }
    else if(n < seaLevel[i] + 0.05) clr = beachColor[i];
    // else if(n < 0.75) clr = vec3(90.,139.,93.) / 255.; else clr = vec3(205., 200., 200.) / 255.; // snow
    else clr = mix(grassColor[i], peakColor[i], smoothstep(seaLevel[i] + 0.07, 0.85, n));
    
    vec2 eps = vec2(0.06, 0.);
    // derivative of implicit surface y = f(x, z) is (-df/dx, 1, -df/dz)
    vec3 sample1 = mtn.xyz + planetBasis * eps.xyy;
    float h1 = noise(normalize(sample1 - spherePos), n <= seaLevel[i] + 0.0001, i);
    vec3 sample1b = mtn.xyz - planetBasis * eps.xyy;
    float h1b = noise(normalize(sample1b - spherePos), n <= seaLevel[i] + 0.0001, i);
    float gradx = (h1 - h1b) / (2. * eps.x);

    vec3 sample2 = mtn.xyz + planetBasis * eps.yyx;
    float h2 = noise(normalize(sample2 - spherePos), n <= seaLevel[i] + 0.0001, i);
    vec3 sample2b = mtn.xyz - planetBasis * eps.yyx;
    float h2b = noise(normalize(sample2b - spherePos), n <= seaLevel[i] + 0.0001, i);
    float gradz = (h2 - h2b) / (2. * eps.x);

    // this only works when the normal is aligned with (Oy), so it needs to be rotated
    // vec3 localNormal = normalize(vec3(-mountainAmplitude * gradx, 1., -mountainAmplitude * gradz));
    vec3 localNormal = normalize(sphereNormal - mountainAmplitude[i] * gradx * normalize(sample1 - sample1b) - mountainAmplitude[i] * gradz * normalize(sample2 - sample2b));

    // no grass grows on slope, but it looked kinda ugly
    // clr = mix(clr, vec3(205., 200., 200.) / 255., 1. - smoothstep(0.0, 0.6, abs(dot(localNormal, sphereNormal))));

    float diffuse = max(-1., dot(localNormal, normalize(lightSource - mtn.xyz)));
    float penumbra = smoothstep(0.1, 0.6, n); // trick for faking global illumination

    float light = max(0., diffuseCoef * diffuse + sphereDiffuse + penumbraCoef * penumbra);
    vec3 shaded = light * clr + ambientCoef * normalize(vec3(1.) + atmosColor[i]);

    refl = shouldReflect;
    return (shouldReflect < -0.1 ? 1. : (1. - shouldReflect)) * shaded;
}

// _____________________________________________________ ATMOSPHERE ________________________________________________________

float densityAtPoint(vec3 where, vec3 planetPos, float planetRadius, int i)
{
    float h = length(where - planetPos) - planetRadius;
    return exp(-h * atmosFalloff[i] / atmosRadius[i]) * (1. - h / atmosRadius[i]);
}

float opticalDepth(vec3 rayDir, vec3 rayPos, float rayLength, float nb_steps, vec3 planetPos, float planetRadius, int i)
{
    vec3 p = rayPos;
    float dt = rayLength / nb_steps;
    float opticalDepth = 0.;

    for(float t = dt; t < rayLength; t += dt)
    {
        p = rayPos + t * rayDir;
        opticalDepth += dt * densityAtPoint(p, planetPos, planetRadius, i);
    }

    return opticalDepth;
}

vec3 atmosphere(vec3 rayDir, vec3 start, float dist, vec3 planetPos, float radius, vec3 lightSource, vec3 originalColor, int i)
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
        float rayLengthToSky = raySphere(p, toLight, planetPos, radius + atmosRadius[i]).y;

        float iOpticalDepth = opticalDepth(toLight, p, rayLengthToSky, NB_STEPS_j, planetPos, radius, i);
        toEyeOpticalDepth = opticalDepth(-rayDir, p, t, NB_STEPS_j, planetPos, radius, i);
        vec3 transmittance = exp(-(iOpticalDepth + toEyeOpticalDepth) * atmosColor[i]);
        float localDensity = densityAtPoint(p, planetPos, radius, i);

        totalLight += localDensity * transmittance * atmosColor[i] * idt;
    }

    float starFade = 3.5 * length(totalLight);
    return totalLight + originalColor * mix(1., exp(-toEyeOpticalDepth), min(1., starFade));
}

// _____________________________________________________ MAIN ________________________________________________________

vec3 raytraceMap(vec3 rayDir, vec3 rayPos)
{   
    vec3 mapColor = vec3(0.);
    vec3 r0 = rayPos, rd = rayDir;
    int NB_MAX_REFLECTIONS = 18;
    float reflectionCoef = 1., nextReflectionCoef = 1.;
    float lastPortal = 0.; // < -1 when blue, > 1 when red
    // no recursivity in GLSL (so we have to use loops for reflection... until I code my own shader language (in some IGR class I hope))
    for(int r = 0; r < NB_MAX_REFLECTIONS; r++)
    {
        bool shouldReflect = false, shouldTeleport = false;

        float tMin = 1e5;
        float tToPlanet = 1e5;
        vec3 argmin = background(rd);

        // planets @here
        for(int i = 0; i < NB_PLANETS; i++)
        {
            vec3 ppi = planetPos[i];
            float pri = uPlanetRadius[i];
            vec2 tPlanet = raySphere(r0, rd, ppi, pri + mountainAmplitude[i]);
            float tstart = max(0., tPlanet.x);
            if(tPlanet.y > tPlanet.x && tstart < tMin && tPlanet.y >= 0.)
            {
                float tOut = 0.;
                float lod = 100. + 600. * (1. - smoothstep(10000., 30000., length(rayPos - cameraPos)));
                lod = (lod / (r + 1.)); // reduce level of detail when looking through recursive portals
                vec3 mountainColor = shadePlanet(rd, r0 + tstart * rd, ppi, 
                                                pri, sunPos, tPlanet.y - tstart, lod, i, nextReflectionCoef, tOut);

                if(mountainColor.x >= -0.1)
                {
                    tMin = tOut + tstart;
                    tToPlanet = tstart;
                    argmin = mountainColor.rgb;

                    shouldReflect = nextReflectionCoef >= -0.1;
                }
                else
                {
                    argmin = background(rd); // this line is cursed, it theoretically does nothing but if you remove it everything breaks
                }
            }

            // atmosphere @here
            vec2 tAtmos = raySphere(r0, rd, ppi, pri + atmosRadius[i]);
            float dstThroughAtmosphere = min(tAtmos.y, tToPlanet - tAtmos.x);
            if(dstThroughAtmosphere > 0.)
            {
                argmin = atmosphere(rd, r0 + tAtmos.x * rd, dstThroughAtmosphere, ppi, pri, sunPos, argmin, i);
            }
        }


        // sun @here
        vec2 corona = raySphere(r0, rd, sunPos, sunCoronaStrength + sunRadius);
        if(corona.y > corona.x && corona.x < tMin && corona.y >= 0.)
        {
            vec2 sun = raySphere(r0, rd, sunPos, sunRadius);
            if(sun.y > sun.x && sun.x < tMin && sun.y >= 0.)
            {
                tMin = sun.x;
                argmin = 2.0 * sunColor;
            }
            else
            {
                float md = (1. / sunRadius) * raySphereMinDist(r0, rd, sunPos, sunRadius).x + 1.;
                float light = smoothstep(0.0, 1.0, 1.0 / (md * md));
                argmin = light * sunColor + (1.0 - light) * argmin;
            }
        }

        // portals @here
        // TODO : move this in a separate function
        vec3 nextr0 = r0;
        vec3 nextrd = rd;
        float tPortal1 = rayCircle(r0, rd, portalPos1, portalPlane1, portalSize1);
        if(tPortal1 <= tMin && (tPortal1 >= 0 || (r == 0 && tPortal1 >= -3.9)))
        {
            nextr0 = portalBasis2 * transpose(portalBasis1) * (r0 + tPortal1 * rd - portalPos1) + portalPos2;
            nextrd = portalBasis2 * transpose(portalBasis1) * rd;
            nextr0 += nextrd * 0.001;

            vec3 locp = r0 + tPortal1 * rd - portalPos1;
            // locp.y *= 0.75;
            float contour = smoothstep(0.9, 0.95, length(locp) / portalSize1);
            argmin = mix(argmin, vec3(0., 0., 1.), contour);

            tMin = tPortal1;
            lastPortal = -10.;
            if(r < NB_MAX_REFLECTIONS - 1 && portalSize2 >= 0.)
            {
                nextReflectionCoef = 1.;
                reflectionCoef = contour;
                shouldTeleport = true;
                shouldReflect = false;
            }
        }
        float tPortal2 = rayCircle(r0, rd, portalPos2, portalPlane2, portalSize2);
        if(tPortal2 <= tMin && (tPortal2 >= 0 || (r == 0 && tPortal2 >= -3.9)))
        {
            nextr0 = portalBasis1 * transpose(portalBasis2) * (r0 + tPortal2 * rd - portalPos2) + portalPos1;
            nextrd = portalBasis1 * transpose(portalBasis2) * rd;
            nextr0 += nextrd * 0.001;

            vec3 locp = r0 + tPortal2 * rd - portalPos2;
            // locp.y *= 0.75;
            float contour = smoothstep(0.9, 0.95, length(locp) / portalSize2);
            argmin = mix(argmin, vec3(1., 0., 0.), contour);

            tMin = tPortal2;
            lastPortal = 10.;
            if(r < NB_MAX_REFLECTIONS - 1 && portalSize1 >= 0.)
            {
                nextReflectionCoef = 1.;
                reflectionCoef = contour;
                shouldTeleport = true;
                shouldReflect = false;
            }
        }

        mapColor += argmin * reflectionCoef; // <----- main render

        if(shouldReflect)
        {
            reflectionCoef = nextReflectionCoef;

            break; // TODO !!!!!
            // float dstToWater = raySphere(r0, rd, planetPos, uPlanetRadius + seaLevel * mountainAmplitude).x;
            // r0 = r0 + dstToWater * rd;
            // rd = reflect(rd, waveNormal(normalize(r0 - planetPos)));
        }
        else if(shouldTeleport)
        {
            reflectionCoef = nextReflectionCoef;
            r0 = nextr0;
            rd = nextrd;
        }
        else
            break;
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
    rayDir = planetBasis * rayDir;

    float distToScreen = length(vec3(uv.x, uv.y, 2. / tan(0.5 * fov)));
    vec3 totalLight = raytraceMap(rayDir, cameraPos + distToScreen * rayDir);

    float edge = length(dFdy(totalLight));
    outColor = vec4(totalLight + 0.55 * edge, 1.0);
}