#version 450

layout(location = 0) in vec2 vFragPos;
layout(location = 0) out vec4 outColor;

uniform float time;
uniform float aspectRatio;
uniform float tCharge;
uniform float tBulletTime;
uniform float tRewind;

float random(float x, float y)
{
    return fract(sin(dot(vec2(x, y), vec2(12.9898, 78.233))) * 43758.5453123);
}

void main()
{
    vec2 uv = vFragPos;
    uv.x *= aspectRatio;
    const float PI = 3.1415926536;
    float theta = atan(-uv.x, -uv.y);

    vec2 shakedUV = uv + 0.02 * pow(tCharge, 2.) * (-1. + 2. * vec2(random(time, time * time), random(random(time, time * time), random(time * time, time))));
    float dashCharge = smoothstep(0.04, 0.05, length(shakedUV)) * smoothstep(-0.07, -0.06, -length(shakedUV)) * step(theta, PI * (2. * tCharge - 1.));
    float center = 1. - step(0.007, length(uv));
    vec4 uiClr = vec4(mix(vec3(1., 1., 0.), vec3(1., 0., 0.), tCharge) * dashCharge + vec3(1.) * center, dashCharge + center);
    float bulletTime = (1. - tBulletTime);
    vec4 bulletTimeClr = smoothstep(0.00001, 0.035, tBulletTime) * vec4(vec3(0.03), bulletTime * 0.5);

    vec3 loopClr = vec3(196., 182., 92.) / 255.;
    vec4 rewindClr = vec4(loopClr, 0.25);

    float n = 30.0;
	float norm_n = pow(pow(abs(vFragPos.x), n) + pow(abs(vFragPos.y), n), 1.0 / n);
    float t = 0.6 * (atan(vFragPos.y, vFragPos.x) - time) + PI;
    float spin = 0.5 + 0.5 * sin(10. * t);
    vec4 loop = 1.4 * smoothstep(0.95, 1., norm_n) * vec4(loopClr * spin, spin * tRewind);

    outColor = vec4(uiClr.rgb * uiClr.a + loop.rgb * loop.a, uiClr.a + loop.a) + bulletTimeClr + tRewind * rewindClr;
}