#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include "math.hpp"

class Camera
{
public:
    Camera(GLFWwindow* __window);
    void update(float dt);

    vec3 getPos() { return pos; }
    vec2 getAngle() { return theta; }
    
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
private:
    GLFWwindow* window{};

    vec3 pos{};
    const float speedRef = 10.0;

    vec2 theta{};
};

#endif // CAMERA_H