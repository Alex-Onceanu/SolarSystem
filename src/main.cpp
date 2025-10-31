#include <glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <array>
#include <memory>

#include "init.h"
#include "input.hpp"
#include "camera.hpp"
#include "math.hpp"

std::array<std::unique_ptr<Planet>, NB_PLANETS> setupPlanets()
{
    std::array<std::unique_ptr<Planet>, NB_PLANETS> res = {
        std::make_unique<Planet>(vec3(-9434.7906, -25662.6391, 2955.8649), 700000000., 500., 64., 0.463, vec4(72., 167., 206., 19.), 8.9, 225., vec3(748., 602., 427.9), vec3(214., 194., 149.), vec3(91., 142., 92.), vec3(205., 215., 195.), 3600., 0.),
        std::make_unique<Planet>(vec3(879.11278, 3896.616536, -1279.07537), 810000000., 608., 89., 0.308, vec4(25., 2., 2., 50.), 9.3, 227., vec3(450., 640., 800.), vec3(164., 80., 80.), vec3(140., 36., 36.), vec3(2., 2., 2.)               , 3600., 100.),
        std::make_unique<Planet>(vec3(13411.7725, 1986.762333, 18793.155), 780000000., 650., 43., 0.098, vec4(97., 131., 146., 2.), 10.5, 292., vec3(475., 536., 800.), vec3(240., 219., 169.), vec3(221., 173., 106.), vec3(242., 165., 55.), 2700., 200.),
        std::make_unique<Planet>(vec3(-27397.563, -27352.0892, -22861.6378), 800000000., 527., 95., 0.355, vec4(138., 211., 193., 7.), 6.48, 244., vec3(461., 400., 507.), vec3(168., 175., 221.), vec3(64., 15., 100.), vec3(135.,106., 183.), 3000., 300.),
        std::make_unique<Planet>(vec3(-6744.2406, -12949.7822, -1483.17318), 950000000., 953., 97., 0.429, vec4(72., 116., 99., 6.), 9.6, 338., vec3(508., 451., 521.), vec3(143., 179., 156.), vec3(159., 211., 158.), vec3(34., 82., 75.)  , 4500., 400.),
        std::make_unique<Planet>(vec3(13625.1274, 51722.1683, -14043.7552), 990000000., 999., 98., 0.973, vec4(63., 155., 222., 2.), 11.6, 303., vec3(752., 607., 441.), vec3(175., 217., 246.), vec3(95., 161., 173.), vec3(43., 127., 215.), 4500., 500.),
        std::make_unique<Planet>(vec3(14351.9012, 11241.1955, 14033.6766), 600000000., 433., 56., 0.655, vec4(9., 6., 49., 12.), 15.8, 322., vec3(400., 400., 400.), vec3(30.,28.,28.), vec3(17., 18., 29.), vec3(0., 0., 0.)                , 2100. ,  600.),
        std::make_unique<Planet>(vec3(0.0, 0.0, 0.0), 630000000., 410., 86., 0.3,   vec4(95., 25., 174., 0.), 10., 263., vec3(508., 555., 530.), vec3(102., 70., 134.), vec3(46., 22., 32.), vec3(176., 232., 244.)         , 2400. , 700.)
    };
    return res;
}

void setPlanetsUniforms(const InputData& inputData, unsigned int program, std::vector<PlanetData> planets)
{
    vec3 planetPos[NB_PLANETS]{};
    float uPlanetRadius[NB_PLANETS]{};
    float mountainAmplitude[NB_PLANETS]{};
    float seaLevel[NB_PLANETS]{};
    vec4 waterColor[NB_PLANETS]{};
    float atmosFalloff[NB_PLANETS]{};
    float atmosRadius[NB_PLANETS]{};
    vec3 atmosColor[NB_PLANETS]{};
    vec3 beachColor[NB_PLANETS]{};
    vec3 grassColor[NB_PLANETS]{};
    vec3 peakColor[NB_PLANETS]{};

    for(int i = 0; i < NB_PLANETS; i++)
    {
        planetPos[i] = planets[i].p;
        uPlanetRadius[i] = planets[i].radius;
        mountainAmplitude[i] = planets[i].mountainAmplitude;
        seaLevel[i] = planets[i].seaLevel;
        waterColor[i] = planets[i].waterColor;
        atmosFalloff[i] = planets[i].atmosFalloff;
        atmosRadius[i] = planets[i].atmosRadius;
        atmosColor[i] = vec3(powf(400. / planets[i].atmosColor.x, 4) * inputData.atmosScattering, powf(400. / planets[i].atmosColor.y, 4) * inputData.atmosScattering, powf(400. / planets[i].atmosColor.z, 4) * inputData.atmosScattering);
        beachColor[i] = planets[i].beachColor;
        grassColor[i] = planets[i].grassColor;
        peakColor[i] = planets[i].peakColor;
    }

    float planetPosLinear[NB_PLANETS * 3];
    int i = 0; 
    while(i < NB_PLANETS * 3) 
    { 
        planetPosLinear[i] = planetPos[i / 3].x; 
        planetPosLinear[i + 1] = planetPos[i / 3].y; 
        planetPosLinear[i + 2] = planetPos[i / 3].z; 
        i += 3;
    }

    float waterColorLinear[NB_PLANETS * 4];
    i = 0; 
    while(i < NB_PLANETS * 4) 
    { 
        waterColorLinear[i] = waterColor[i / 4].x / 255.; 
        waterColorLinear[i + 1] = waterColor[i / 4].y / 255.; 
        waterColorLinear[i + 2] = waterColor[i / 4].z / 255.; 
        waterColorLinear[i + 3] = waterColor[i / 4].w / 255.; 
        i += 4;
    }

    float atmosColorLinear[NB_PLANETS * 3];
    i = 0; 
    while(i < NB_PLANETS * 3) 
    { 
        atmosColorLinear[i] = atmosColor[i / 3].x; 
        atmosColorLinear[i + 1] = atmosColor[i / 3].y; 
        atmosColorLinear[i + 2] = atmosColor[i / 3].z; 
        i += 3;
    }

    float beachColorLinear[NB_PLANETS * 3];
    i = 0; 
    while(i < NB_PLANETS * 3) 
    { 
        beachColorLinear[i] = beachColor[i / 3].x / 255.; 
        beachColorLinear[i + 1] = beachColor[i / 3].y / 255.; 
        beachColorLinear[i + 2] = beachColor[i / 3].z / 255.; 
        i += 3;
    }
    float grassColorLinear[NB_PLANETS * 3];
    i = 0; 
    while(i < NB_PLANETS * 3) 
    { 
        grassColorLinear[i] = grassColor[i / 3].x / 255.; 
        grassColorLinear[i + 1] = grassColor[i / 3].y / 255.; 
        grassColorLinear[i + 2] = grassColor[i / 3].z / 255.; 
        i += 3;
    }
    float peakColorLinear[NB_PLANETS * 3];
    i = 0; 
    while(i < NB_PLANETS * 3) 
    { 
        peakColorLinear[i] = peakColor[i / 3].x / 255.; 
        peakColorLinear[i + 1] = peakColor[i / 3].y / 255.; 
        peakColorLinear[i + 2] = peakColor[i / 3].z / 255.; 
        i += 3;
    }

    glUniform3fv(glGetUniformLocation(program, "planetPos"), NB_PLANETS, planetPosLinear);
    glUniform1fv(glGetUniformLocation(program, "uPlanetRadius"), NB_PLANETS, uPlanetRadius);
    glUniform1fv(glGetUniformLocation(program, "mountainAmplitude"), NB_PLANETS, mountainAmplitude);
    glUniform1fv(glGetUniformLocation(program, "seaLevel"), NB_PLANETS, seaLevel);
    glUniform4fv(glGetUniformLocation(program, "waterColor"), NB_PLANETS, waterColorLinear);
    glUniform1fv(glGetUniformLocation(program, "atmosFalloff"), NB_PLANETS, atmosFalloff);
    glUniform1fv(glGetUniformLocation(program, "atmosRadius"), NB_PLANETS, atmosRadius);
    glUniform3fv(glGetUniformLocation(program, "atmosColor"), NB_PLANETS, atmosColorLinear);
    glUniform3fv(glGetUniformLocation(program, "beachColor"), NB_PLANETS, beachColorLinear);
    glUniform3fv(glGetUniformLocation(program, "grassColor"), NB_PLANETS, grassColorLinear);
    glUniform3fv(glGetUniformLocation(program, "peakColor"), NB_PLANETS, peakColorLinear);
}

int main()
{
    GLFWwindow* window = nullptr;
    unsigned int program = init(&window);
    unsigned int UIprogram = initUI();

    // auto earthTexture = init_texture("../assets/eart.ppm");
    auto opticalDepthTexture = init_texture("../assets/noise.pgm");

    Input::init(window);
    auto camera = std::make_unique<Camera>(window, vec3(-9434.7906 - 300, -25662.6391 + 600, 2955.8649));

    auto startTime = std::chrono::high_resolution_clock::now();
    auto prevTime = startTime;
    auto lastSecondTime = startTime;

    glUniform1f(glGetUniformLocation(program, "aspectRatio"), static_cast<float>(RESOLUTION_W) / static_cast<float>(RESOLUTION_H));
    glUseProgram(UIprogram);
    glUniform1f(glGetUniformLocation(UIprogram, "aspectRatio"), static_cast<float>(RESOLUTION_W) / static_cast<float>(RESOLUTION_H));
    glUseProgram(program);

    unsigned int frameBuf, outTexture;
    generateLowResBuf(&frameBuf, &outTexture);

    // Otherwise we see a pink screen as very first frame when launching the program
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    auto planets = setupPlanets();
    float time = 0.;

    // mainloop here
    while (!glfwWindowShouldClose(window))
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        // realTime will only be used for UI and "real world" durations (such as cooldowns)
        float realTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
        prevTime = currentTime;
        time += dt; // time is sum of dt so that we can slow time and rewind it

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, earthTexture);
        // glUniform1i(glGetUniformLocation(program, "earthTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, opticalDepthTexture);
        glUniform1i(glGetUniformLocation(program, "heightmap"), 1);

        auto inputData = Input::getInput();
        glUniform1f(glGetUniformLocation(program, "time"), time);
        glUniform3f(glGetUniformLocation(program, "sunPos"), 
            inputData.sunPos[0], inputData.sunPos[1], inputData.sunPos[2]);
        glUniform1f(glGetUniformLocation(program, "sunRadius"), inputData.sunRadius);
        glUniform3f(glGetUniformLocation(program, "sunColor"), 
            inputData.sunColor[0], inputData.sunColor[1], inputData.sunColor[2]);
        glUniform1f(glGetUniformLocation(program, "sunCoronaStrength"), 
            inputData.sunCoronaStrength);
        glUniform1f(glGetUniformLocation(program, "fov"), inputData.fov * 3.1415 / 180.);

        glUniform1f(glGetUniformLocation(program, "NB_STEPS_i"), inputData.nb_steps_i);
        glUniform1f(glGetUniformLocation(program, "NB_STEPS_j"), inputData.nb_steps_j);
        glUniform1f(glGetUniformLocation(program, "refractionindex"), inputData.refractionindex);
        glUniform1f(glGetUniformLocation(program, "fresnel"), inputData.fresnel);

        glUniform1f(glGetUniformLocation(program, "ambientCoef"), inputData.ambientCoef);
        glUniform1f(glGetUniformLocation(program, "diffuseCoef"), inputData.diffuseCoef);
        glUniform1f(glGetUniformLocation(program, "minDiffuse"), inputData.minDiffuse);
        glUniform1f(glGetUniformLocation(program, "penumbraCoef"), inputData.penumbraCoef);

        glUniform1f(glGetUniformLocation(program, "nbStars"), inputData.nbStars);
        glUniform1f(glGetUniformLocation(program, "starsDisplacement"), inputData.starsDisplacement);
        glUniform1f(glGetUniformLocation(program, "starSize"), inputData.starSize);
        glUniform1f(glGetUniformLocation(program, "starSizeVariation"), inputData.starSizeVariation);
        glUniform1f(glGetUniformLocation(program, "starVoidThreshold"), inputData.starVoidThreshold);
        glUniform1f(glGetUniformLocation(program, "starFlickering"), inputData.starFlickering);

        camera->setSpeedRef(inputData.cameraSpeed);
        camera->setJumpStrength(inputData.jumpStrength);
        std::vector<PlanetData> pdv;
        for(int i = 0; i < NB_PLANETS; i++)
        {
            pdv.push_back(planets[i]->getInfo());
        }
        camera->update(dt, realTime, pdv);
        setPlanetsUniforms(inputData, program, pdv);

        for(const auto& e : planets)
        {
            e->update(dt);
        }

        vec3 camPos = camera->getPos();
        glUniform3f(glGetUniformLocation(program ,"cameraPos"), camPos.x, camPos.y, camPos.z);

        vec2 camTheta = camera->getAngle();
        glUniform2f(glGetUniformLocation(program ,"cameraRotation"), camTheta.x, camTheta.y);

        float lb[9];
        camera->getPlanetBasis(lb);
        glUniformMatrix3fv(glGetUniformLocation(program ,"planetBasis"), 1, false, lb);

        vec3 portalPlane1{}, portalPlane2{};
        vec3 portalPos1{}, portalPos2{};
        float portalSize1 = -1., portalSize2 = -1.;
        float pb1[9], pb2[9];
        camera->getPortalInfo(portalPlane1, portalPlane2, portalPos1, portalPos2, portalSize1, portalSize2, pb1, pb2);
        glUniform3f(glGetUniformLocation(program ,"portalPlane1"), portalPlane1.x, portalPlane1.y, portalPlane1.z);
        glUniform3f(glGetUniformLocation(program ,"portalPlane2"), portalPlane2.x, portalPlane2.y, portalPlane2.z);
        glUniform3f(glGetUniformLocation(program ,"portalPos1"), portalPos1.x, portalPos1.y, portalPos1.z);
        glUniform3f(glGetUniformLocation(program ,"portalPos2"), portalPos2.x, portalPos2.y, portalPos2.z);
        glUniform1f(glGetUniformLocation(program ,"portalSize1"), portalSize1);
        glUniform1f(glGetUniformLocation(program ,"portalSize2"), portalSize2);
        glUniformMatrix3fv(glGetUniformLocation(program ,"portalBasis1"), 1, false, pb1);
        glUniformMatrix3fv(glGetUniformLocation(program ,"portalBasis2"), 1, false, pb2);

        int W, H;
        glfwGetWindowSize(window, &W, &H);

        // Main render pass @here
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuf);
        glViewport(0, 0, LOW_RES_W, LOW_RES_H);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuf);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, LOW_RES_W, LOW_RES_H, 0, 0, W, H, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Draw UI over the framebuffer
        glUseProgram(UIprogram);
        glUniform1f(glGetUniformLocation(UIprogram, "time"), time);
        glUniform1f(glGetUniformLocation(UIprogram, "tCharge"), camera->getDashTimer());
        glUniform1f(glGetUniformLocation(UIprogram, "tBulletTime"), camera->getBulletTimer());
        glUniform1f(glGetUniformLocation(UIprogram, "tRewind"), camera->isRewinding() ? 1. : 0.);
        
        glViewport(0, 0, W, H);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        glUseProgram(program);

        Input::renderInterface();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    Input::destroy();

    return 0;
}