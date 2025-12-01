#include "camera.hpp"
#include "math.hpp"

#include <iostream>
#include <algorithm>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

constexpr size_t NB_KEYS = 8;

bool isKeyPressed[NB_KEYS];
bool justClicked = false, justUnclicked = false, rightClicking = false, justRightClicked = false;
bool bluePortalPressed = false, redPortalPressed = false;
bool shouldHideCursor = false;
vec2 mousePos;

// found this value noise on https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
// is actually a low-frequency version of the heightmap used for rendering the mountains, for collisions
float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - vec4::floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + vec4(1.,1.,1.,1.)) * x);}
float noisep(vec3 p)
{
    vec3 a = vec3::floor(p);
    vec3 d = p - a;
    d = d * d * (vec3(3.,3.,3.) - d * 2.);

    vec4 b = vec4(a.x, a.x, a.y, a.y) + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(vec4(b.x, b.y, b.x, b.y));
    vec4 k2 = perm(vec4(k1.x, k1.y, k1.x, k1.y) + vec4(b.z, b.z, b.w, b.w));

    vec4 c = k2 + vec4(a.z, a.z, a.z, a.z);
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + vec4(1.,1.,1.,1.));

    vec4 o1 = vec4::fract(k3 * (1.0 / 41.0));
    vec4 o2 = vec4::fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = vec2(o3.y, o3.w) * d.x + vec2(o3.x, o3.z) * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

float Camera::noise(const vec3& uvw) const
{
    return std::max(seaLevel, noisep(uvw * 8.));
}

float Camera::heightHere(const PlanetData& pl) const
{
    float mountainHeight = mountainAmplitude * noise((pos - pl.p).normalize());
    return pl.radius + 65. + mountainHeight;
}


EM_BOOL mouse_button_callback(int eventType, const EmscriptenMouseEvent* e, void* userData)
{
    if(shouldHideCursor)
    {
        if(e->button == 0) // Bouton gauche
        {
            if(eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
                justClicked = true;
            else if(eventType == EMSCRIPTEN_EVENT_MOUSEUP)
                justUnclicked = true;
        }
        else if(e->button == 2) // Bouton droit
        {
            if(eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
                justRightClicked = true;

            rightClicking = (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN);
        }
    }

    return EM_TRUE;
}

EM_BOOL request_pointerlock(int eventType, const EmscriptenMouseEvent* e, void* userData)
{
    if(!shouldHideCursor)
    {
        shouldHideCursor = emscripten_request_pointerlock("#canvas", EM_TRUE) == EMSCRIPTEN_RESULT_SUCCESS;
    }
    return EM_TRUE;
}

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
    bool isPressed = (eventType == EMSCRIPTEN_EVENT_KEYDOWN);

    // Remplacement des touches GLFW par leurs noms JS correspondants
    const char* everyKey[NB_KEYS] = { "w", "a", "s", "d", " " }; // " " pour espace
    const char* altKeys[NB_KEYS]   = { "z", "q", nullptr, nullptr, nullptr };

    for(int i = 0; i < NB_KEYS; i++)
    {
        if(strcmp(e->key, everyKey[i]) == 0 || (altKeys[i] && strcmp(e->key, altKeys[i]) == 0))
        {
            isKeyPressed[i] = isPressed;
        }
    }

    bluePortalPressed = (strcmp(e->key, "e") == 0 && isPressed);
    redPortalPressed  = (strcmp(e->key, "r") == 0 && isPressed);

    return EM_TRUE;
}


EM_BOOL mouse_move_callback(int eventType, const EmscriptenMouseEvent* e, void* userData)
{
    if(!shouldHideCursor) return EM_TRUE;

    mousePos.x = static_cast<float>(e->movementX);
    mousePos.y = static_cast<float>(e->movementY);

    return EM_TRUE;
}

void Camera::updatePlanetBasis(const PlanetData& closest)
{
    normal = (pos - closest.p).normalize();
    backRef = (backRef - normal * backRef.dot(normal)).normalize();
    leftRef = normal.cross(backRef);
}

void Camera::walk(const float dt, const PlanetData& closest, bool ignoreKeys)
{
    vec3 stepSpeed{};

    if(!ignoreKeys)
    {
        if(isKeyPressed[0]) stepSpeed.z -= 1.0;
        if(isKeyPressed[2]) stepSpeed.z += 1.0;

        if(isKeyPressed[1]) stepSpeed.x -= 1.0;
        if(isKeyPressed[3]) stepSpeed.x += 1.0;

        stepSpeed.normalized();
        stepSpeed *= speedRef;
    }
    // relative to when the normal is (0, 1, 0)
    back = vec3(sinf(theta.x) * cosf(theta.y), -sinf(theta.y), -cosf(theta.x) * cosf(theta.y));
    left = vec3(cosf(theta.x), 0., sinf(theta.x)) * -1.;
    up = back.cross(left);

    // change of basis
    back  = vec3( back.dot(vec3(-leftRef.x, normal.x, backRef.x)),  back.dot(vec3(-leftRef.y, normal.y, backRef.y)),  back.dot(vec3(-leftRef.z, normal.z, backRef.z)));
    left  = vec3( left.dot(vec3(-leftRef.x, normal.x, backRef.x)),  left.dot(vec3(-leftRef.y, normal.y, backRef.y)),  left.dot(vec3(-leftRef.z, normal.z, backRef.z)));
    up    = vec3(   up.dot(vec3(-leftRef.x, normal.x, backRef.x)),    up.dot(vec3(-leftRef.y, normal.y, backRef.y)),    up.dot(vec3(-leftRef.z, normal.z, backRef.z)));

    float dstToCtr = (pos - closest.p).length();
    pos -= (left - normal * normal.dot(left)).normalize() * stepSpeed.x * dt;
    pos += (back - normal * normal.dot(back)).normalize() * stepSpeed.z * dt;
    pos = (pos - closest.p).normalize() * dstToCtr + closest.p;
    pos += up * stepSpeed.y * dt;
}

void Camera::updateMouse()
{
    EmscriptenPointerlockChangeEvent e;
    if(emscripten_get_pointerlock_status(&e) == EMSCRIPTEN_RESULT_SUCCESS)
      shouldHideCursor = e.isActive;


    if(justRightClicked)
    {
        startedRewinding = time;
        justRightClicked = false;
        rewindingStart = timeline->front();
    }
    else if(justClicked)
    {
        dashStartTime = time;
        justClicked = false;
        charging = true;
    }
    else if(justUnclicked)
    {
        dash();
        justUnclicked = false;
        charging = false;
        tDash = 0.;
    }

    if(prevWouldHideCursor)
    {
        theta.x -= mousePos.x * (0.00032f * 3.141592653589793f);
        if(theta.x <= -3.141592653589793f) theta.x += 2. * 3.141592653589793f;
        if(theta.x >= 3.141592653589793f) theta.x -= 2. * 3.141592653589793f;
        theta.y -= mousePos.y * (0.00032f * 3.141592653589793f);
        theta.y = std::max(-3.141592653589793f / 2.0f + 0.0001f, std::min(theta.y, 3.141592653589793f / 2.0f - 0.0001f));
        rewinding = rightClicking;
    }
    prevWouldHideCursor = shouldHideCursor;
    mousePos = vec2();
}

PlanetData Camera::findClosest(const std::vector<PlanetData>& planets)
{
    PlanetData closest = { .p = vec3(INFINITY, INFINITY, INFINITY), .radius = 1., .mass = 1. };
    for(int i = 0; i < planets.size(); i++)
    {
        if((planets[i].p - pos).length() < (closest.p - pos).length()) 
        {
            closest = planets[i];
            iClosest = i;
        }

        // update portalclosest @here
        if(iPortalClosest1 != -1 and iPortalClosest1 == i)
        {
            dposForPortal1 = planets[i].p - oldClosestPosForPortal1;
            oldClosestPosForPortal1 = planets[i].p;
        }
        if(iPortalClosest2 != -1 and iPortalClosest2 == i)
        {
            dposForPortal2 = planets[i].p - oldClosestPosForPortal2;
            oldClosestPosForPortal2 = planets[i].p;
        }
    }
    return closest;
}

void Camera::applyGravity(const float& dt, const PlanetData& closest)
{
    onGround = false;
    vec3 Fdir = (pos - closest.p).normalize();
    vec3 F = Fdir * (-closest.mass / (closest.p - pos).dot(closest.p - pos));
    gravitySpeed += F * dt; // Newton's second law + integrating acceleration
    float h = heightHere(closest);
    if((closest.p - pos).length() <= h)
    {
        onGround = true;
        gravitySpeed = vec3(); // reset gravity when landing
        pos = (pos - closest.p).normalize() * h + closest.p;
        dashSpeed -= normal * normal.dot(dashSpeed); // project dash speed on normal plane (slide)
    }
}

Camera::Camera(vec3 spawn)
    : pos(spawn)
{
    timeline = std::make_unique<std::deque<vec3>>();
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, key_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, key_callback);
    emscripten_set_mousemove_callback("#canvas", nullptr, EM_TRUE, mouse_move_callback);
    emscripten_set_mousedown_callback("#canvas", nullptr, EM_TRUE, mouse_button_callback);
    emscripten_set_mouseup_callback("#canvas", nullptr, EM_TRUE, mouse_button_callback);

    emscripten_set_mousedown_callback("#canvas", nullptr, EM_TRUE, request_pointerlock);
}


void Camera::jump(const float dt)
{
    gravitySpeed += normal * (jumpStrength);
}

void Camera::dash()
{
    const float dashStrength = 1700.;
    vec3 F = back * (-getDashTimer() * dashStrength);
    dashSpeed = F;
    gravitySpeed = vec3();
    onGround = false;
    tDash = 0.;
}

void Camera::bluePortal()
{
    portalPlane1 = back;
    portalPos1 = pos - back * distToPortal;
    portalSize1 = 0.0001;
    tPortalAnim1 = 0.0001;
    bluePortalPressed = false;
    portalBasis1.C1 = left * -1., portalBasis1.C2 = up, portalBasis1.C3 = back;

    if((pos - oldClosestPos).length() < 650. + closest.radius + closest.mountainAmplitude)
    {
        iPortalClosest1 = iClosest;
        oldClosestPosForPortal1 = oldClosestPos;
    }
    else iPortalClosest1 = -1;
}

void Camera::redPortal()
{
    portalPlane2 = back;
    portalPos2 = pos - back * distToPortal;
    portalSize2 = 0.0001;
    tPortalAnim2 = 0.0001;
    redPortalPressed = false;
    portalBasis2.C1 = left * -1., portalBasis2.C2 = up, portalBasis2.C3 = back;

    if((pos - oldClosestPos).length() < 650. + closest.radius + closest.mountainAmplitude)
    {
        iPortalClosest2 = iClosest;
        oldClosestPosForPortal2 = oldClosestPos;
    }
    else iPortalClosest2 = -1;
}

// same function is used for rendering the portals (see main.frag)
// returns 1e6 if no intersection
float rayCircle(vec3 rayPos, vec3 rayDir, vec3 cPos, vec3 cPlane, float radius)
{
    if((fabsf(rayDir.dot(cPlane)) <= 0.00001f) || radius < 0.) return 1e6;
    float t = (cPos.dot(cPlane) - rayPos.dot(cPlane)) / rayDir.dot(cPlane);
    vec3 p = rayPos + rayDir * t - cPos;
    if(p.length() > radius + 4.) return 1e6;
    return t;
}

bool Camera::wentThroughPortal(const vec3& plane, const vec3& center, const float& size) const
{
    if((plane.dot(oldPos - center) >= 0.) == (plane.dot(pos - center) >= 0.)) return false;
    vec3 rd = (pos - oldPos).normalize();
    return rayCircle(oldPos, rd, center, plane, size) < (1e6 - 1.);
}

void Camera::teleportThroughPortal(const PlanetData& closest)
{
    if(portalSize1 < 0. or portalSize2 < 0.) return;
    vec3 rd = (pos - oldPos).normalize();
    if(wentThroughPortal(portalPlane1, portalPos1, portalSize1) and portalCooldown <= 0)
    {
        pos = portalBasis2 * portalBasis1.transpose() * (pos - portalPos1) + portalPos2;
        dashSpeed = portalBasis2 * portalBasis1.transpose() * dashSpeed;
        gravitySpeed = portalBasis2 * portalBasis1.transpose() * gravitySpeed;
        backRef = portalBasis2 * portalBasis1.transpose() * backRef;
        portalCooldown = 20;
    }
    if(wentThroughPortal(portalPlane2, portalPos2, portalSize2) and portalCooldown <= 0)
    {
        pos = portalBasis1 * portalBasis2.transpose() * (pos - portalPos2) + portalPos1;
        dashSpeed = portalBasis1 * portalBasis2.transpose() * dashSpeed;
        gravitySpeed = portalBasis1 * portalBasis2.transpose() * gravitySpeed;
        backRef = portalBasis2 * portalBasis1.transpose() * backRef;
        portalCooldown = 20;
    }
}

void Camera::update(float& dt, const float& __time, const std::vector<PlanetData>& planets)
{
    time = __time;
    
    int iOldClosest = iClosest;
    closest = findClosest(planets);
    mountainAmplitude = closest.mountainAmplitude;
    seaLevel = closest.seaLevel;
    if(iOldClosest != -1)
    {
        if(iOldClosest != iClosest)
        {
            // change of closest planet
            vec3 oldBack = back, oldLeft = left, oldNormal = normal;
            updatePlanetBasis(closest);
            thetaDiff = acosf(oldNormal.dot(normal));
            animAxis = (normal.cross(oldNormal)).normalize();
            oldTheta = theta;
            newTheta = vec2(acosf(oldLeft.dot(leftRef)), -asinf(normal.dot(oldBack)));
            animStart = time;
            changingPlanet = true;
        }
        else if((pos - closest.p).length() < 600. + closest.radius + closest.mountainAmplitude + 65. and not rewinding)
        {
            pos += closest.p - oldClosestPos;
        }
    }
    oldClosestPos = closest.p;

    if(iPortalClosest1 != -1) portalPos1 += dposForPortal1;
    if(iPortalClosest2 != -1) portalPos2 += dposForPortal2;

    updateMouse();

    if(changingPlanet)
    {
        if(time - animStart > animDuration)
        {
            theta = newTheta;
            backRef = initBackRef.rotate(animAxis, thetaDiff);
            leftRef = initLeftRef.rotate(animAxis, thetaDiff);
            normal = initNormal.rotate(animAxis, thetaDiff);
            changingPlanet = false;
        }
        else
        {
            float tt = (time - animStart) / animDuration;
            theta = oldTheta * (1.f - tt) + newTheta * tt;
            backRef = initBackRef.rotate(animAxis, thetaDiff * tt);
            leftRef = initLeftRef.rotate(animAxis, thetaDiff * tt);
            normal = initNormal.rotate(animAxis, thetaDiff * tt);
        }
    }

    if(rewinding and not timeline->empty())
    {
        dt *= -1.;
        dashSpeed = vec3();
        gravitySpeed = vec3();
        float rewindingSince = 10. * (time - startedRewinding);
        float t = 1. - CLAMP(rewindingSince - static_cast<float>(static_cast<int>(rewindingSince)), 0.f, 1.f); // = fract(rewindingSince)
        vec3 end = timeline->front();
        pos = rewindingStart * t + end * (1. - t); // lerp position between current checkpoint and previous one
        // updatePlanetBasis(closest);

        auto tick = static_cast<unsigned int>(rewindingSince); // = floor(rewindingSince)
        if(tick != lastRewindingTick)
        {
            // next tick : pop the dequeue (next checkpoint)
            rewindingStart = timeline->front();
            pos = rewindingStart;
            timeline->pop_front();
            lastRewindingTick = tick;
        }
    }
    else if(charging) // bullet time baby
    {
        tDash = CLAMP(time - dashStartTime, 0.f, bulletTimeDuration);
        dt *= std::max(0.1f, getBulletTimer() * getBulletTimer());
    }

    auto tick = static_cast<unsigned int>(10. * time); // tick is floor(time) (10 ticks per second)
    if(tick != lastTimelineTick and not rewinding)
    {
        const size_t MAX_NB_TICKS = 10000;
        if(timeline->size() >= MAX_NB_TICKS)
            timeline->pop_back();

        timeline->push_front(pos);
        lastTimelineTick = tick;
    }

    applyGravity(dt, closest);

    if(portalCooldown > 0) portalCooldown--;
    if(portalSize1 > 0. and tPortalAnim1 < 1.)
    {
        tPortalAnim1 = CLAMP(tPortalAnim1 + dt / portalAnimTime, 0.f, 1.f);
        portalSize1 = powf(tPortalAnim1, 1. / 4.) * portalSizeRef;
    }
    if(portalSize2 > 0. and tPortalAnim2 < 1.)
    {
        tPortalAnim2 = CLAMP(tPortalAnim2 + dt / portalAnimTime, 0.f, 1.f);
        portalSize2 = powf(tPortalAnim2, 1. / 4.) * portalSizeRef;
    }

    // if((pos - closest.p).length() <= heightHere(closest) + mountainAmplitude) 
    walk(dt, closest, rewinding);
    if(not rewinding)
    {
        if(bluePortalPressed) bluePortal();
        if(redPortalPressed) redPortal();

        teleportThroughPortal(closest);
        oldPos = pos;
        if(isKeyPressed[4] and onGround) jump(dt);
    }
    else oldPos = pos;
    updatePlanetBasis(closest);

    pos += dashSpeed * dt; // position is integral of speed
    pos += gravitySpeed * dt;
}