#include "camera.hpp"
#include "math.hpp"

#include "imgui.h"

#include <iostream>
#include <algorithm>

constexpr float PI = 3.141592f;
constexpr size_t NB_KEYS = 7;

bool isKeyPressed[NB_KEYS];
bool shouldHideCursor = true;
vec2 mousePos;

Camera::Camera(GLFWwindow* __window, vec3 spawn, vec3 firstPlanet)
    : window(__window), pos(spawn)
{
    up = (spawn - firstPlanet).normalize();
    right = (right - up * up.dot(right)).normalize();
    front = up.cross(right);

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
    if(!isKeyPressed[4] and shouldHideCursor) return;

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, action);
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
        mousePos.x = static_cast<float>(xpos), mousePos.y = static_cast<float>(ypos);
}

void Camera::update(float dt, std::vector<PlanetData> planets)
{
    if(isKeyPressed[4] or not shouldHideCursor)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // if(glfwRawMouseMotionSupported())
        // { 
        //     glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        // }
        glfwSetCursorPos(window, 0., 0.);
    }

    // find closest planet
    PlanetData closest = { .p = vec3(INFINITY, INFINITY, INFINITY), .radius = 1., .mass = 1. };
    for(auto e : planets)
    {
        if((e.p - pos).length() < (closest.p - pos).length()) 
            closest = e;
    }

    float dstToCtr = (closest.p - pos).length();

    // std::cout << (closest.p - pos).length() << std::endl;

    // const float playerHeight = 30.;
    // vec3 Fdir = pos - closest.p;
    // Fdir.normalized();
    // vec3 F = Fdir * (-closest.mass / (closest.p - pos).dot(closest.p - pos));
    // if((closest.p - pos).length() > closest.radius + playerHeight + 50.)
    //     speed += F * dt;
    // else
    //     speed = vec3(0., 0., 0.);


    vec3 tmpSpeed(0., 0., 0.);

    // this code is right-handed, but OpenGl's NDC are left-handed for some reason
    if(isKeyPressed[0]) tmpSpeed.z += 1.0;
    if(isKeyPressed[2]) tmpSpeed.z -= 1.0;

    if(isKeyPressed[1]) tmpSpeed.x -= 1.0;
    if(isKeyPressed[3]) tmpSpeed.x += 1.0;
    if(isKeyPressed[5]) tmpSpeed.y += 1.0;
    if(isKeyPressed[6]) tmpSpeed.y -= 1.0;

    tmpSpeed.normalized();
    tmpSpeed *= speedRef;

    // if(tmpSpeed.length() > 0.01) speed = vec3(0., 0., 0.);

    // vec3 direction = vec3(sinf(theta.x) * cosf(theta.y), -sinf(theta.y), -cosf(theta.x) * cosf(theta.y));
    // vec3 right = vec3(cosf(theta.x), 0., sinf(theta.x));
    // vec3 up = right.cross(direction);

    theta.x -= mousePos.x * (0.02f * PI * dt);
    theta.y -= mousePos.y * (0.02f * PI * dt);
    theta.y = std::max(-PI / 2.0f + 0.0001f, std::min(theta.y, PI / 2.0f - 0.0001f));
    mousePos = vec2(0.0, 0.0);

    // rotate camera basis (only front and right) based on planet size
    float ftsx = fabsf(tmpSpeed.x), ftsz = fabsf(tmpSpeed.z);
    if(ftsz >= 0.0001)
    {
        pos = (pos - closest.p).rotate(right * (dt * ftsz / dstToCtr), tmpSpeed.z < 0.) + closest.p;
        up = (pos - closest.p).normalize();
        front = front.rotate(right * (dt * ftsz / dstToCtr), tmpSpeed.z < 0.);
    }
    if(ftsx >= 0.0001)
    {
        pos = (pos - closest.p).rotate(front * (dt * ftsx / dstToCtr), tmpSpeed.x >= 0.) + closest.p;
        up = (pos - closest.p).normalize();
        right = right.rotate(front * (dt * ftsx / dstToCtr), tmpSpeed.x >= 0.);
    }

    std::cout << dstToCtr << std::endl;

    // std::cout << "front : " << direction << "\nright : " << right << "\nup : " << up << std::endl;
    // speed = (right * tmpSpeed.x + up * tmpSpeed.y + front * tmpSpeed.z);

    pos += speed * dt;
}