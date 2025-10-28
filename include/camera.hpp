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
    Camera(GLFWwindow* __window, vec3 spawn);
    void update(float& dt, const float& __time, const std::vector<PlanetData>& planets);

    vec3 getPos() { return pos; }
    vec2 getAngle() { return theta; }
    void getPlanetBasis(float v[9]) { if(!v) return; 
        v[0] =-leftRef.x;    v[3] = normal.x;   v[6] = frontRef.x; 
        v[1] =-leftRef.y;    v[4] = normal.y;   v[7] = frontRef.y; 
        v[2] =-leftRef.z;    v[5] = normal.z;   v[8] = frontRef.z; }
    
    void setSpeedRef(const float& v) { speedRef = v; }
    void setJumpStrength(const float& v) { jumpStrength = v; }
    void setMountainParams(const float& mountainAmp, const float& sea) { mountainAmplitude = mountainAmp; seaLevel = sea; }

    float getDashTimer() { return std::min(dashCharge, tDash) / dashCharge; }
    float getBulletTimer() { return tDash / bulletTimeDuration; }
    bool isRewinding() { return rewinding; }

    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void glfwCharCallback(GLFWwindow* window, unsigned int c);

private:
    void walk(const float dt, const PlanetData& closest);
    void jump(const float dt);
    void dash();
    void updateMouse();
    PlanetData findClosest(const std::vector<PlanetData>& planets);
    void applyGravity(const float& dt, const std::vector<PlanetData>& planets);
    void updatePlanetBasis(const PlanetData& closest);
    float heightHere(const PlanetData& pl) const;
    float noise(const vec3& uvw) const;

private:
    GLFWwindow* window{};

    float time{};
    vec3 pos{};
    vec3 speed{};
    vec3 gravitySpeed{};

    float speedRef{};
    bool onGround = false;
    float jumpStrength{};

    float mountainAmplitude{}, seaLevel{};

    vec3 normal = vec3(0., 1., 0.), frontRef = vec3(0., 0., 1.), leftRef = vec3(-1., 0., 0.);
    vec3 front = frontRef;
    vec2 theta{};

    float dashStartTime{};
    bool charging = false;
    const float dashCharge = 1.5;
    const float bulletTimeDuration = 4.;
    float tDash = 0.; // 0 when not charging, 1 when full charging

    bool rewinding = false;
};

#endif // CAMERA_H