#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>

#include <vector>

#include "math.hpp"

struct PlanetData
{
    vec3 p;
    float radius;
    float mass;
};

class Camera
{
public:
    Camera(GLFWwindow* __window);
    void update(float dt, std::vector<PlanetData> planets);

    vec3 getPos() { return pos; }
    vec2 getAngle() { return theta; }
    
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void glfwCharCallback(GLFWwindow* window, unsigned int c);


private:
    GLFWwindow* window{};

    vec3 pos{};
    vec3 speed{};
    const float speedRef = 200.0;

    vec2 theta{};
};

#endif // CAMERA_H