#include "camera.hpp"
#include "math.hpp"

#include "imgui.h"

#include <iostream>
#include <algorithm>

constexpr size_t NB_KEYS = 8;

bool isKeyPressed[NB_KEYS];
bool justClicked = false, justUnclicked = false, rightClicking = false, justRightClicked = false;
bool bluePortalPressed = false, redPortalPressed = false;
bool shouldHideCursor = true;
vec2 mousePos;

Camera::Camera(GLFWwindow* __window, vec3 spawn)
    : window(__window), pos(spawn)
{
    timeline = std::make_unique<std::deque<vec3>>();
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    glfwSetCharCallback(window, glfwCharCallback);

    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(glfwRawMouseMotionSupported())
    { 
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

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
    const float playerHeight = 65.;
    float mountainHeight = mountainAmplitude * noise((pos - pl.p).normalize());
    return pl.radius + playerHeight + mountainHeight;
}

void Camera::glfwCharCallback(GLFWwindow* window, unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

void Camera::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if(!isKeyPressed[4] and shouldHideCursor)
    {
        if(button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_PRESS)
            justClicked = true;

        else if(button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_RELEASE)
            justUnclicked = true;

        if(button == GLFW_MOUSE_BUTTON_RIGHT and action == GLFW_PRESS)
            justRightClicked = true;

        rightClicking = (button == GLFW_MOUSE_BUTTON_RIGHT and action == GLFW_PRESS or action == GLFW_REPEAT);   
    }
    else
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(button, action);
    }
}

void Camera::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{   
    bool isPressed = action == GLFW_PRESS || action == GLFW_REPEAT;

    int everyKey[NB_KEYS]{ GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_ALT, GLFW_KEY_SPACE };
    int altKeys[NB_KEYS] { GLFW_KEY_Z, GLFW_KEY_Q, -1, -1, GLFW_KEY_RIGHT_ALT, -1 };

    for(int i = 0; i < NB_KEYS; i++)
    {
        if(key == everyKey[i] || (altKeys[i] != -1 && key == altKeys[i]))
        {
            isKeyPressed[i] = isPressed;
        }
    }
    if(key == GLFW_KEY_ESCAPE and action == GLFW_PRESS)
    {
        shouldHideCursor = not shouldHideCursor;
        glfwSetCursorPos(window, 0.0, 0.0);
    }

    bluePortalPressed = (key == GLFW_KEY_E and action == GLFW_PRESS);
    redPortalPressed = (key == GLFW_KEY_R and action == GLFW_PRESS);
}

void Camera::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    if(isKeyPressed[4] or not shouldHideCursor)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(xpos, ypos);
    }
    else 
    {
        mousePos.x = static_cast<float>(xpos), mousePos.y = static_cast<float>(ypos);
    }
}

void Camera::updatePlanetBasis(const PlanetData& closest)
{
    normal = (pos - closest.p).normalize();
    backRef = (backRef - normal * backRef.dot(normal)).normalize();
    leftRef = normal.cross(backRef);
}

void Camera::walk(const float dt, const PlanetData& closest)
{
    vec3 stepSpeed{};

    if(isKeyPressed[0]) stepSpeed.z -= 1.0;
    if(isKeyPressed[2]) stepSpeed.z += 1.0;

    if(isKeyPressed[1]) stepSpeed.x -= 1.0;
    if(isKeyPressed[3]) stepSpeed.x += 1.0;

    stepSpeed.normalized();
    stepSpeed *= speedRef;

    // relative to when the normal is (0, 1, 0)
    back = vec3(sinf(theta.x) * cosf(theta.y), -sinf(theta.y), -cosf(theta.x) * cosf(theta.y));
    left = vec3(cosf(theta.x), 0., sinf(theta.x)) * -1.;
    up = back.cross(left);

    // change of basis
    back = vec3(back.dot(vec3(-leftRef.x, normal.x, backRef.x)), back.dot(vec3(-leftRef.y, normal.y, backRef.y)), back.dot(vec3(-leftRef.z, normal.z, backRef.z)));
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
    if(isKeyPressed[4] or not shouldHideCursor)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, 0., 0.);
    }

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

    theta.x -= mousePos.x * (0.00032f * M_PIf);
    theta.y -= mousePos.y * (0.00032f * M_PIf);
    theta.y = std::max(-M_PIf / 2.0f + 0.0001f, std::min(theta.y, M_PIf / 2.0f - 0.0001f));
    mousePos = vec2();
    rewinding = rightClicking;
}

PlanetData Camera::findClosest(const std::vector<PlanetData>& planets)
{
    PlanetData closest = { .p = vec3(INFINITY, INFINITY, INFINITY), .radius = 1., .mass = 1. };
    for(const auto& e : planets)
    {
        if((e.p - pos).length() < (closest.p - pos).length()) 
            closest = e;
    }
    return closest;
}

void Camera::applyGravity(const float& dt, const std::vector<PlanetData>& planets)
{
    onGround = false;
    for(const auto& e : planets)
    {
        vec3 Fdir = (pos - e.p).normalize();
        vec3 F = Fdir * (-e.mass / (e.p - pos).dot(e.p - pos));
        gravitySpeed += F * dt; // Newton's second law + integrating acceleration
        float h = heightHere(e);
        if((e.p - pos).length() <= h)
        {
            onGround = true;
            gravitySpeed = vec3(); // reset gravity when landing
            pos = (pos - e.p).normalize() * h + e.p;
            dashSpeed -= normal * normal.dot(dashSpeed); // project dash speed on normal plane (slide)
            break;
        }
    }
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
    portalSize1 = 60.; // TODO : Tween size for portal spawn
    bluePortalPressed = false;
    portalBasis1.C1 = left * -1., portalBasis1.C2 = up, portalBasis1.C3 = back;
}

void Camera::redPortal()
{
    portalPlane2 = back; // TODO : back instead of backref and view matrix instead of planetBasis
    portalPos2 = pos - back * distToPortal;
    portalSize2 = 60.; // TODO : Tween size for portal spawn
    redPortalPressed = false;
    portalBasis2.C1 = left * -1., portalBasis2.C2 = up, portalBasis2.C3 = back;
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

void Camera::teleportThroughPortal()
{
    if(portalSize1 < 0. or portalSize2 < 0.) return;
    if(wentThroughPortal(portalPlane1, portalPos1, portalSize1))
    {
        pos = portalBasis2 * portalBasis1.transpose() * (pos - portalPos1) + portalPos2;
        dashSpeed = portalBasis2 * portalBasis1.transpose() * dashSpeed;
        gravitySpeed = portalBasis2 * portalBasis1.transpose() * gravitySpeed;
    }
    else if(wentThroughPortal(portalPlane2, portalPos2, portalSize2))
    {
        pos = portalBasis1 * portalBasis2.transpose() * (pos - portalPos2) + portalPos1;
        dashSpeed = portalBasis1 * portalBasis2.transpose() * dashSpeed;
        gravitySpeed = portalBasis1 * portalBasis2.transpose() * gravitySpeed;
    }
}

void Camera::update(float& dt, const float& __time, const std::vector<PlanetData>& planets)
{
    time = __time;
    updateMouse();
    PlanetData closest = findClosest(planets);
    if(rewinding and not timeline->empty())
    {
        // dt *= -1.;
        dashSpeed = vec3();
        gravitySpeed = vec3();
        float rewindingSince = 10. * (time - startedRewinding);
        float t = 1. - CLAMP(rewindingSince - static_cast<float>(static_cast<int>(rewindingSince)), 0.f, 1.f); // = fract(rewindingSince)
        vec3 end = timeline->front();
        pos = rewindingStart * t + end * (1. - t); // lerp position between current checkpoint and previous one
        updatePlanetBasis(closest);

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

    updatePlanetBasis(closest);
    applyGravity(dt, planets);
    // if((pos - closest.p).length() <= heightHere(closest) + mountainAmplitude) 
    if(not rewinding)
    {
        if(bluePortalPressed) bluePortal();
        if(redPortalPressed) redPortal();

        teleportThroughPortal();
        updatePlanetBasis(closest);

        walk(dt, closest);
        if(isKeyPressed[5] and onGround) jump(dt);
    }

    oldPos = pos;
    pos += dashSpeed * dt; // position is integral of speed
    pos += gravitySpeed * dt;
}