#version 450

layout(location = 0) in vec2 vFragPos;
layout(location = 0) out vec4 outColor;

uniform float time;
uniform vec3 cameraPos;
uniform vec2 cameraRotation;

uniform vec3 sunPos;
uniform vec3 sunColor;
uniform float sunCoronaStrength;
uniform float aspectRatio;

uniform vec3 planetPos;
uniform vec3 planetColor;

uniform float noiseScale;

mat2 rot2D(float theta)
{
    return mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
}

float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}
float fbm(vec3 p)
{
    int nb_octaves = 3;
    float amp = 64.0;
    float ampsum = 0.;
    float freq = 1.0;

    float ans = 0.;
    for(int i = 0; i < nb_octaves; i++)
    {
        ampsum += amp;
        ans += amp * noise(p * freq);
        amp /= 2.;
        freq *= 2.;
    }
    return ans / ampsum;
}

// Returns .x > .y if no intersection
vec2 raySphere(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius)
{
    vec3 p = rayPos - sphPos;
    float delta = 4. * (dot(p, rayDir) * dot(p, rayDir) - dot(rayDir, rayDir) * (dot(p, p) - radius * radius));
    if(delta < 0.) return vec2(1., -1.);
    return vec2((-dot(p, rayDir) - sqrt(delta)) / dot(rayDir, rayDir), 
                (-dot(p, rayDir) + sqrt(delta)) / dot(rayDir, rayDir));
}

float raySphereMinDist(vec3 rayPos, vec3 rayDir, vec3 spherePos, float radius)
{
    float t = -dot(rayDir, rayPos - spherePos) / dot(rayDir, rayDir);
    if(t <= 0.) return 1e5;
    vec3 pos = rayPos + t * rayDir;
    return length(pos - spherePos) - radius;
}

// vec3 cross(vec3 a, vec3 b)
// {
//     return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
// }

vec3 background(vec3 rayDir)
{
    // Distant stars here
    return vec3(0.);
    // return vec3(rayDir.xy, 0.);
    // return vec3(fract(0.5 * normalize(rayDir)).xy, 0.);
}

float rayCastMountains(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius)
{
    float nb_iterations = 300.;
    float maxt = 50.;//raySphere(rayPos, rayDir, sphPos, radius + 1.).y;
    float dt = 0.1;//max(0.05, maxt / nb_iterations);
    float lh = 0.0;
    float ly = 0.0;
    for(float t = 0.001; t < maxt; t += dt)
    {
        vec3 p = rayPos + t * rayDir;
        float py = length(p - sphPos) - radius;
        float h = fbm(noiseScale * p);
        if(py < h)
        {
            return t-dt+dt*(lh-ly)/(py-ly-h+lh);
        }
        lh = h;
        ly = py;
    }
    return -1.;
}

// float planetDist(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius)
// {
//     float tImpact = raySphere(rayPos, rayDir, sphPos, radius).x;
//     vec3 impactPos = rayPos + tImpact * rayDir;
//     float height = fbm(normalize(impactPos - sphPos) * noiseScale);

//     return raySphere(rayPos, rayDir, sphPos, radius + height).x;
// }

vec3 shadePlanet(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius, vec3 lightSource)
{
    // float tMountains = planetDist(rayPos, rayDir, sphPos, radius);
    // vec3 pos = rayPos + tMountains * rayDir;
    // vec2 e = vec2(0.2, 0.0);
    // vec3 tNormal = vec3(planetDist(rayPos + e.xyy, rayDir, sphPos, radius).x,
    //                     planetDist(rayPos + e.yxy, rayDir, sphPos, radius).x,
    //                     planetDist(rayPos + e.yyx, rayDir, sphPos, radius).x);

    // vec3 normal = normalize(vec3(tMountains) - tNormal);
    // vec3 toLight = normalize(pos - lightSource);

    // float h = length(pos - sphPos) - radius;

    // vec3 clr;

    // if(h < 0.4) clr = vec3(79., 76., 176.) / 255.;
    // else if(h < 0.45) clr = vec3(216., 197., 150.) / 255.;
    // else if(h < 0.65) clr = vec3(159., 193., 100.) / 255.;
    // else clr = vec3(195.,146.,79.) / 255.;

    // return max(0., dot(normal, toLight)) * clr;

    float t = rayCastMountains(rayPos, rayDir, sphPos, radius);
    if(t < 0.) return vec3(-1.);

    vec3 pos = rayPos + t * rayDir;
    vec3 clr;
    float n = length(pos - sphPos) - radius;

    if(n < 0.4) clr = vec3(79., 76., 176.) / 255.;
    else if(n < 0.45) clr = vec3(216., 197., 150.) / 255.;
    else if(n < 0.65) clr = vec3(159., 193., 100.) / 255.;
    else clr = vec3(195.,146.,79.) / 255.;

    return clr;

    // clr = vec3(159., 193., 100.) / 255.;

    // vec3 toLight = normalize(lightSource - pos);
    // float darkSide = rayCastMountains(pos + 0.01 * normalize(pos - sphPos), toLight, sphPos, radius);
    // if(darkSide >= 0.) return 0.2 * clr;
    // return clr;

    // float here = fbm(pos);
    // float right = fbm(noiseScale * (normalize(pos - sphPos) + vec3(0.01, 0., 0.)));
    // float up = fbm(noiseScale * (normalize(pos - sphPos) + vec3(0., 0., 0.01)));
    // vec2 grad = vec2(right - here, up - here) / 0.01;
    // vec3 normal = normalize(vec3(-grad.x, 1., -grad.y));

    // return max(0., dot(normal, toLight)) * clr;
}

vec3 raytraceMap(vec3 rayPos, vec3 rayDir)
{
    const int NB_PLANETS = 1;
    vec3 planets[NB_PLANETS] = { planetPos };

    float tMin = 1e5;
    vec3 argmin = background(rayDir);

    float md = raySphereMinDist(rayPos, rayDir, sunPos, 1.0) + 1.;
    argmin += sunColor * smoothstep(0.0, 1.0, 1.0 / (md * md));

    for(int i = 0; i < NB_PLANETS; i++)
    {
        vec2 planet = raySphere(rayPos, rayDir, planets[i], 5.0);
        if(planet.y > planet.x)
        {
            float t = planet.x;
            if(t < tMin && t >= 0.)
            {
                vec3 planetShading = shadePlanet(rayPos, rayDir, planets[i], 4.0, sunPos);
                if(planetShading.x > 0.)
                {
                    tMin = t;
                    argmin = planetShading;
                }
            }
        }
    }

    vec2 sun = raySphere(rayPos, rayDir, sunPos, 1.0);
    if(sun.y > sun.x)
    {
        float t = sun.x;
        if(t < tMin && t >= 0.)
        {
            tMin = t;
            argmin = sunCoronaStrength * sunColor;
        }
    }

    return argmin;
}

void main()
{
    vec2 uv = vFragPos;
    uv.x *= aspectRatio;

    vec3 rayDir = normalize(vec3(uv.x, uv.y, 6.0));
    rayDir.yz *= rot2D(cameraRotation.y);
    rayDir.xz *= rot2D(-cameraRotation.x);

    vec3 totalLight = raytraceMap(cameraPos, rayDir);
    
    outColor = vec4(totalLight, 1.0);
}