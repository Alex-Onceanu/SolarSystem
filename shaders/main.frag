#version 450

layout(location = 0) in vec3 vFragColor;
layout(location = 0) out vec4 outColor;

bool equals(vec3 a, vec3 b)
{
    float epsilon = 0.001;
    return dot(b - a, b - a) <= epsilon;
}

float raySphereIntersection(vec3 rayDir, vec3 rayPos, vec3 sph, float radius)
{
    vec3 p = rayPos - sph;
    return 4. * (dot(p, rayDir) * dot(p, rayDir) - dot(rayDir, rayDir) * (dot(p, p) - radius * radius));
}

vec3 posFromDelta(float delta, vec3 rayDir, vec3 rayPos, vec3 sph)
{
    vec3 p = rayPos - sph;
    float t = -(dot(p, rayDir) + sqrt(delta)) / (dot(rayDir, rayDir));
    return rayPos + t * rayDir;
}

vec3 intersectSphere(vec3 rayDir, vec3 rayPos, vec3 spherePos, float radius)
{
    float delta = raySphereIntersection(rayDir, rayPos, spherePos, radius);
    if(delta >= 0.0)
    {
        return posFromDelta(delta, rayDir, rayPos, spherePos);
    }
    return rayPos;
}

float raySphereMinDist(vec3 rayDir, vec3 rayPos, vec3 spherePos, float radius)
{
    float t = -dot(rayDir, rayPos - spherePos) / dot(rayDir, rayDir);
    vec3 pos = rayPos + t * rayDir;
    return length(pos - spherePos) - radius;
}

vec3 shadeSphere(vec3 rayDir, vec3 rayPos, vec3 spherePos, float radius, vec3 lightSource, vec3 clr)
{
    vec3 pos = intersectSphere(rayDir, rayPos, spherePos, radius);
    if(equals(pos, rayPos))
    {
        float md = raySphereMinDist(rayDir, rayPos, spherePos, radius) + 1.;
        return clr * smoothstep(0.0, 1.0, 1.0 / (md * md));
    }
    return clr;//max(dot(normalize(pos - spherePos), normalize(lightSource - pos)), 0.0) * 0.8 * clr + 0.2 * clr;
}

void main() {
    vec2 uv = vFragColor.xy;
    uv.x *= 1.777778; // aspect ratio

    vec3 lightSource = vec3(6.0, 5.0, -3.5);

    vec3 totalLight = vec3(0.0);
    vec3 spherePos = vec3(0.0, 0.0, 5.0);

    vec3 cameraPos = vec3(0.0, 0.0, -2.0);
    vec3 rayDir = vec3(uv.x, uv.y, 0.0) - cameraPos;

    vec3 sphereSurface = intersectSphere(rayDir, cameraPos, spherePos, 1.0);
    totalLight += shadeSphere(rayDir, cameraPos, spherePos, 1.0, lightSource, vec3(4.0, 4.0, 3.5));
    
    outColor = vec4(totalLight, 1.0);
}