#version 450

layout(location = 0) in vec2 vFragPos;
layout(location = 0) out vec4 outColor;

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

mat2 rot2D(float theta)
{
    return mat2(vec2(cos(theta), -sin(theta)), vec2(sin(theta), cos(theta)));
}

// Returns .x > .y if no intersection
vec2 raySphere(vec3 rayPos, vec3 rayDir, vec3 sphPos, float radius)
{
    vec3 p = rayPos - sphPos;
    float delta = 4. * (dot(p, rayDir) * dot(p, rayDir) - dot(rayDir, rayDir) * (dot(p, p) - radius * radius));
    if(delta < 0.) return vec2(1., -1.);
    return vec2((-2. * dot(p, rayDir) - sqrt(delta)) / (2. * dot(rayDir, rayDir)), 
                (-2. * dot(p, rayDir) + sqrt(delta)) / (2. * dot(rayDir, rayDir)));
}

float raySphereMinDist(vec3 rayPos, vec3 rayDir, vec3 spherePos, float radius)
{
    float t = -dot(rayDir, rayPos - spherePos) / dot(rayDir, rayDir);
    if(t <= 0.) return 1e5;
    vec3 pos = rayPos + t * rayDir;
    return length(pos - spherePos) - radius;
}

vec3 shadePlanet(vec3 rayDir, vec3 pos, vec3 spherePos, float radius, vec3 lightSource, vec3 clr)
{
    vec3 d = (pos - spherePos) / radius;
    vec2 uv = vec2(0.5 + atan(d.z, d.x) / (2. * 3.1416), 0.5 - asin(d.y) / 3.1416);

    return max(0., dot(d, normalize(lightSource - pos))) * texture(earthTexture, uv).gbr;
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

    // float md = raySphereMinDist(rayPos, rayDir, sunPos, 1.0) + 1.;
    // argmin += sunColor * smoothstep(0., 1.0, 1.0 / (md * md));

    for(int i = 0; i < NB_PLANETS; i++)
    {
        vec2 planet = raySphere(rayPos, rayDir, planets[i], 10.0);
        if(planet.y > planet.x)
        {
            float t = planet.x;
            if(t < tMin && t >= 0.)
            {
                tMin = t;
                argmin = shadePlanet(rayDir, rayPos + t * rayDir, planets[i], 10.0, sunPos, planetColor);
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

    vec3 rayDir = vec3(uv.x, uv.y, 2. / tan(0.5 * fov));
    rayDir.yz *= rot2D(-cameraRotation.y);
    rayDir.xz *= rot2D(cameraRotation.x);

    float distToScreen = length(vec3(uv.x, uv.y, 2. / tan(0.5 * fov)));
    vec3 totalLight = raytraceMap(rayDir, cameraPos + distToScreen * rayDir);
    
    outColor = vec4(totalLight, 1.0);
}