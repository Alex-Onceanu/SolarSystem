#version 450

layout(location = 0) in vec2 vFragPos;
layout(location = 0) out vec4 outColor;

uniform vec3 cameraPos;
uniform vec2 cameraRotation;

uniform vec3 sunPos;
uniform vec3 sunColor;
uniform float sunCoronaStrength;
uniform float aspectRatio;

uniform vec3 planetPos;
uniform vec3 planetColor;

mat2 rot2D(float theta)
{
    return mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
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
    vec3 pos = rayPos + t * rayDir;
    return length(pos - spherePos) - radius;
}

vec3 shadePlanet(vec3 rayDir, vec3 pos, vec3 spherePos, float radius, vec3 lightSource, vec3 clr)
{
    return max(dot(normalize(pos - spherePos), normalize(lightSource - pos)), 0.0) * 0.8 * clr + 0.2 * clr;
}

vec3 background(vec3 rayDir)
{
    // Distant stars here
    
    return vec3(0., 0., 0.);
}

vec3 raytraceMap(vec3 rayDir, vec3 rayPos)
{   
    const int NB_PLANETS = 1;
    vec3 planets[NB_PLANETS] = { planetPos };

    float tMin = 1e5;
    vec3 argmin = background(rayDir);

    float md = raySphereMinDist(rayPos, rayDir, sunPos, 1.0) + 1.;
    argmin += sunColor * smoothstep(0.0, 1.0, 1.0 / (md * md));

    for(int i = 0; i < NB_PLANETS; i++)
    {
        vec2 planet = raySphere(rayPos, rayDir, planets[i], 1.0);
        if(planet.y > planet.x)
        {
            float t = planet.x;
            if(t < tMin && t >= 0.)
            {
                tMin = t;
                argmin = shadePlanet(rayDir, rayPos + t * rayDir, planets[i], 1.0, sunPos, planetColor);
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

    vec3 rayDir = normalize(vec3(uv.x, uv.y, 2.0));
    rayDir.yz *= rot2D(cameraRotation.y);
    rayDir.xz *= rot2D(-cameraRotation.x);

    vec3 totalLight = raytraceMap(rayDir, cameraPos);
    
    outColor = vec4(totalLight, 1.0);
}