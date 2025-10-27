#include "camera.hpp"
#include "math.hpp"

#include "imgui.h"

#include <iostream>
#include <algorithm>

constexpr size_t NB_KEYS = 7;

bool isKeyPressed[NB_KEYS];
bool isClicked = false;
bool shouldHideCursor = true;
vec2 mousePos;

Camera::Camera(GLFWwindow* __window, vec3 spawn)
    : window(__window), pos(spawn)
{
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
        {
            isClicked = true;
        }
        else if(button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_RELEASE)
        {
            isClicked = false; // peut mieux faire ?
        }
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

    int everyKey[NB_KEYS]{ GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_ALT, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT };
    int altKeys[NB_KEYS] { GLFW_KEY_UP, GLFW_KEY_LEFT, GLFW_KEY_DOWN, GLFW_KEY_RIGHT, GLFW_KEY_RIGHT_ALT, -1, -1 };

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
    frontRef = (frontRef - normal * frontRef.dot(normal)).normalize();
    leftRef = normal.cross(frontRef);
}

void Camera::walk(const float dt, const PlanetData& closest)
{
    vec3 stepSpeed{};

    if(isKeyPressed[0]) stepSpeed.z -= 1.0;
    if(isKeyPressed[2]) stepSpeed.z += 1.0;

    if(isKeyPressed[1]) stepSpeed.x -= 1.0;
    if(isKeyPressed[3]) stepSpeed.x += 1.0;
    // if(isKeyPressed[5]) stepSpeed.y += 1.0; 
    // if(isKeyPressed[6]) stepSpeed.y -= 1.0;

    stepSpeed.normalized();
    stepSpeed *= speedRef;

    // relative to when the normal is (0, 1, 0)
    vec3 front = vec3(sinf(theta.x) * cosf(theta.y), -sinf(theta.y), -cosf(theta.x) * cosf(theta.y));
    vec3 left = vec3(cosf(theta.x), 0., sinf(theta.x)) * -1.;
    vec3 up = front.cross(left);

    // change of basis
    front = vec3(front.dot(vec3(-leftRef.x, normal.x, frontRef.x)), front.dot(vec3(-leftRef.y, normal.y, frontRef.y)), front.dot(vec3(-leftRef.z, normal.z, frontRef.z)));
    left  = vec3( left.dot(vec3(-leftRef.x, normal.x, frontRef.x)),  left.dot(vec3(-leftRef.y, normal.y, frontRef.y)),  left.dot(vec3(-leftRef.z, normal.z, frontRef.z)));
    up    = vec3(   up.dot(vec3(-leftRef.x, normal.x, frontRef.x)),    up.dot(vec3(-leftRef.y, normal.y, frontRef.y)),    up.dot(vec3(-leftRef.z, normal.z, frontRef.z)));

    float dstToCtr = (pos - closest.p).length();
    pos -= (left - normal * normal.dot(left)).normalize() * stepSpeed.x * dt;
    pos += (front - normal * normal.dot(front)).normalize() * stepSpeed.z * dt;
    pos = (pos - closest.p).normalize() * dstToCtr + closest.p;
    pos += up * stepSpeed.y * dt;
}

void Camera::updateMouse(const float dt)
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

    theta.x -= mousePos.x * (0.00032f * M_PIf);
    theta.y -= mousePos.y * (0.00032f * M_PIf);
    theta.y = std::max(-M_PIf / 2.0f + 0.0001f, std::min(theta.y, M_PIf / 2.0f - 0.0001f));
    mousePos = vec2();
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
        speed += F * dt;
        float h = heightHere(e);
        if((e.p - pos).length() <= h)
        {
            onGround = true;
            speed = vec3();
            pos = (pos - e.p).normalize() * h + e.p;
            break;
        }
    }
}

float Camera::noise(const vec3& uvw) const
{
    vec2 uv = vec2(0.5 + atan2(uvw.z, uvw.x) / (2. * M_PIf), 0.5 - asin(uvw.y) / M_PIf);
    uv.x *= mountainTextureSize.x, uv.y *= mountainTextureSize.y;
    float h = static_cast<float>(mountainTexture[static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    h += static_cast<float>(mountainTexture[1 + static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    h += static_cast<float>(mountainTexture[-1 + static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    h += static_cast<float>(mountainTexture[static_cast<int>(mountainTextureSize.x) + static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    h += static_cast<float>(mountainTexture[-static_cast<int>(mountainTextureSize.x) + static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    h += static_cast<float>(mountainTexture[1 + static_cast<int>(mountainTextureSize.x) + static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    h += static_cast<float>(mountainTexture[-1 + static_cast<int>(mountainTextureSize.x) + static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    h += static_cast<float>(mountainTexture[1 - static_cast<int>(mountainTextureSize.x) + static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    h += static_cast<float>(mountainTexture[-1 -static_cast<int>(mountainTextureSize.x) + static_cast<int>(uv.y * mountainTextureSize.x + uv.x)]) / 255.f;
    return std::max(seaLevel, h / 9.f);
}

float Camera::heightHere(const PlanetData& pl) const
{
    const float playerHeight = 80.;

    float mountainHeight = mountainAmplitude * noise((pos - pl.p).normalize());

    return pl.radius + playerHeight + mountainAmplitude; // TODO : activate this
}

void Camera::jump(const float dt)
{
    if(not onGround) return;
    speed += normal * jumpStrength;
}

void Camera::update(float& dt, const std::vector<PlanetData>& planets)
{
    updateMouse(dt);
    applyGravity(dt, planets);

    if(isKeyPressed[5]) jump(dt);

    PlanetData closest = findClosest(planets);
    updatePlanetBasis(closest);

    // if(onGround) 
    walk(dt, closest);
    
    pos += speed * dt; // Newton's second law
}