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
    void getPlanetBasis(float v[9]) { if(!v) return; 
        v[0] =-leftRef.x;    v[3] = normal.x;   v[6] = frontRef.x; 
        v[1] =-leftRef.y;    v[4] = normal.y;   v[7] = frontRef.y; 
        v[2] =-leftRef.z;    v[5] = normal.z;   v[8] = frontRef.z; }
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

    vec3 normal = vec3(0., 1., 0.), frontRef = vec3(0., 0., 1.), leftRef = vec3(-1., 0., 0.);
    

    vec2 theta{};
};

#endif // CAMERA_H