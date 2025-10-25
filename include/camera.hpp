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
    Camera(GLFWwindow* __window, vec3 spawn, vec3 firstPlanet);
    void update(float dt, std::vector<PlanetData> planets);

    vec3 getPos() { return pos; }
    vec2 getAngle() { return theta; }
    void getView(float* v) { if(!v) return; 
        v[0] =viewLeft.x;  v[3] =viewLeft.y;  v[6] =viewLeft.z; 
        v[1] = viewUp.x;    v[4] = viewUp.y;    v[7] = viewUp.z; 
        v[2] =lookat.x;    v[5] = lookat.y;   v[8] =lookat.z; }
    void setSpeedRef(const float& v) { speedRef = v; }
    
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void glfwCharCallback(GLFWwindow* window, unsigned int c);


private:
    GLFWwindow* window{};

    vec3 pos{};
    vec3 speed{};
    float speedRef{};

    vec3 front = vec3(0., 0., 1.), left = vec3(-1., 0., 0.), up = vec3(0., 1., 0.);
    vec3 lookat = front, viewLeft = left, viewUp = up;

    vec2 theta{};
};

#endif // CAMERA_H