#ifndef PLANET_H
#define PLANET_H

#include "math.hpp"
#include "input.hpp"

struct PlanetData
{
    vec3 p;
    float radius;
    float mass;

    float mountainAmplitude;
    float seaLevel;
    vec4 waterColor;
    float atmosFalloff;
    float atmosRadius;
    vec3 atmosColor;

    vec3 beachColor;
    vec3 grassColor;
    vec3 peakColor;
};

class Planet
{
public:
    Planet(const vec3& __pos, const float& __mass, const float& __radius, 
        const float& __mountainAmplitude, const float& __seaLevel, const vec4& __waterColor, 
        const float& __atmosFalloff, const float& __atmosRadius, const vec3& __atmosColor, 
        const vec3& __beachColor, const vec3& __grassColor, const vec3& __peakColor, 
        const float& __periodDuration, const float& __phase);
    
    PlanetData getInfo() const;

    void update(const float& dt);

private:
    vec3 pos{};
    float mass = 1.;
    float radius = 1.;

    const vec3 sunPos = vec3(0.,30.,10360.);
    vec3 otherFocal{};
    float periodDuration{};
    float phase{};

    float mountainAmplitude;
    float seaLevel;
    vec4 waterColor;
    float atmosFalloff;
    float atmosRadius;
    vec3 atmosColor;

    vec3 beachColor;
    vec3 grassColor;
    vec3 peakColor;
};

#endif