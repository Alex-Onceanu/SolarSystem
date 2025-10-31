#include "planet.hpp"
#include "input.hpp"


Planet::Planet( const vec3& __pos, const float& __mass, const float& __radius, 
                const float& __mountainAmplitude, const float& __seaLevel, const vec4& __waterColor, 
                const float& __atmosFalloff, const float& __atmosRadius, const vec3& __atmosColor, 
                const vec3& __beachColor, const vec3& __grassColor, const vec3& __peakColor,
                const float& __periodDuration, const float& __phase )
    : pos(__pos), mass(__mass), radius(__radius), mountainAmplitude(__mountainAmplitude), 
        seaLevel(__seaLevel), waterColor(__waterColor), atmosFalloff(__atmosFalloff),
        atmosRadius(__atmosRadius), atmosColor(__atmosColor),
        beachColor(__beachColor), grassColor(__grassColor), peakColor(__peakColor),
        periodDuration(__periodDuration), phase(__phase)
{
    pos -= sunPos;
    pos.y = cosf(phase) * pos.y - sinf(phase) * pos.z;
    pos.z = sinf(phase) * pos.y + cosf(phase) * pos.z;
    pos += sunPos;
}

PlanetData Planet::getInfo() const
{
    PlanetData ans{
        .p = pos, .radius = radius, .mass = mass,
        .mountainAmplitude = mountainAmplitude, .seaLevel = seaLevel, .waterColor = waterColor,
        .atmosFalloff = atmosFalloff, .atmosRadius = atmosRadius, .atmosColor = atmosColor,
        .beachColor = beachColor, .grassColor = grassColor, .peakColor = peakColor
    };

    return ans;
}

void Planet::update(const float& dt)
{
    float dphi = 2. * M_PIf * dt / periodDuration;
    pos -= sunPos;
    pos.y = cosf(dphi) * pos.y - sinf(dphi) * pos.z;
    pos.z = sinf(dphi) * pos.y + cosf(dphi) * pos.z;
    pos += sunPos;
}