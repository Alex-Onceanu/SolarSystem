#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>

#include <vector>
#include <deque>
#include <memory>

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
        v[0] =-leftRef.x;    v[3] = normal.x;   v[6] = backRef.x; 
        v[1] =-leftRef.y;    v[4] = normal.y;   v[7] = backRef.y; 
        v[2] =-leftRef.z;    v[5] = normal.z;   v[8] = backRef.z; }

    void getPortalInfo(vec3& pportalPlane1, vec3& pportalPlane2, vec3& pportalPos1, vec3& pportalPos2, float& pportalSize1, float& pportalSize2, float* pb1, float* pb2) const
    {
        pportalPlane1 = portalPlane1; pportalPlane2 = portalPlane2;
        pportalPos1 = portalPos1; pportalPos2 = portalPos2;
        pportalSize1 = portalSize1; pportalSize2 = portalSize2;

        pb1[0] =-portalBase1[0].x;    pb1[3] = portalBase1[1].x;   pb1[6] = portalBase1[2].x; 
        pb1[1] =-portalBase1[0].y;    pb1[4] = portalBase1[1].y;   pb1[7] = portalBase1[2].y;
        pb1[2] =-portalBase1[0].z;    pb1[5] = portalBase1[1].z;   pb1[8] = portalBase1[2].z;

        pb2[0] =-portalBase2[0].x;    pb2[3] = portalBase2[1].x;   pb2[6] = portalBase2[2].x; 
        pb2[1] =-portalBase2[0].y;    pb2[4] = portalBase2[1].y;   pb2[7] = portalBase2[2].y; 
        pb2[2] =-portalBase2[0].z;    pb2[5] = portalBase2[1].z;   pb2[8] = portalBase2[2].z;
    }


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
    void bluePortal();
    void redPortal();

private:
    GLFWwindow* window{};

    float time{};
    vec3 pos{};
    vec3 dashSpeed{};
    vec3 gravitySpeed{};

    float speedRef{};
    bool onGround = false;
    float jumpStrength{};

    float mountainAmplitude{}, seaLevel{};

    vec3 normal = vec3(0., 1., 0.), backRef = vec3(0., 0., 1.), leftRef = vec3(-1., 0., 0.);
    vec3 back = backRef;
    vec2 theta{};

    float dashStartTime{};
    bool charging = false;
    const float dashCharge = 1.5;
    const float bulletTimeDuration = 4.;
    float tDash = 0.; // 0 when not charging, 1 when full charging

    std::unique_ptr<std::deque<vec3>> timeline;
    unsigned int lastTimelineTick = 0;
    bool rewinding = false;
    float startedRewinding = 0.;
    int lastRewindingTick = 0;
    vec3 rewindingStart{};

    const float distToPortal = 150.;
    vec3 portalPlane1{}, portalPlane2{};
    vec3 portalPos1{}, portalPos2{};
    float portalSize1 = -1., portalSize2 = -1.;
    vec3 portalBase1[3]{}, portalBase2[3]{};
};

#endif // CAMERA_H