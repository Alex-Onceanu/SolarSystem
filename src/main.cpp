#include <glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <memory>

#include "init.h"
#include "input.hpp"
#include "camera.hpp"
#include "math.hpp"

int main()
{
    GLFWwindow* window = nullptr;
    unsigned int program = init(&window);
    unsigned int UIprogram = initUI();

    auto earthTexture = init_texture("../assets/eart.ppm");
    auto opticalDepthTexture = init_texture("../assets/noise.pgm");

    Input::init(window);
    auto camera = std::make_unique<Camera>(window, vec3(0., 350.0, 300.0));

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

    float time = 0.;

    // mainloop here
    while (!glfwWindowShouldClose(window))
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        // realTime will only be used for UI and "real world" durations
        float realTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
        prevTime = currentTime;
        time += dt; // time is sum of dt so that we can rewind time and morph it

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        glUniform1i(glGetUniformLocation(program, "earthTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, opticalDepthTexture);
        glUniform1i(glGetUniformLocation(program, "opticalDepthTexture"), 1);

        auto inputData = Input::getInput();
        glUniform1f(glGetUniformLocation(program, "time"), time);
        glUniform3f(glGetUniformLocation(program, "sunPos"), 
            inputData.sunPos[0], inputData.sunPos[1], inputData.sunPos[2]);
        glUniform1f(glGetUniformLocation(program, "sunRadius"), inputData.sunRadius);
        glUniform3f(glGetUniformLocation(program, "sunColor"), 
            inputData.sunColor[0], inputData.sunColor[1], inputData.sunColor[2]);
        glUniform1f(glGetUniformLocation(program, "sunCoronaStrength"), 
            inputData.sunCoronaStrength);
        glUniform3f(glGetUniformLocation(program, "planetPos"), 
            inputData.planetPos[0], inputData.planetPos[1], inputData.planetPos[2]);
        glUniform1f(glGetUniformLocation(program, "uPlanetRadius"), inputData.planetRadius);
        glUniform1f(glGetUniformLocation(program, "fov"), inputData.fov * 3.1415 / 180.);

        glUniform1f(glGetUniformLocation(program, "NB_STEPS_i"), inputData.nb_steps_i);
        glUniform1f(glGetUniformLocation(program, "NB_STEPS_j"), inputData.nb_steps_j);
        glUniform1f(glGetUniformLocation(program, "atmosRadius"), inputData.atmosRadius);
        glUniform1f(glGetUniformLocation(program, "atmosFalloff"), inputData.atmosFalloff);
        glUniform3f(glGetUniformLocation(program, "atmosColor"), 
            powf(400. / inputData.atmosColor[0], 4) * inputData.atmosScattering, 
            powf(400. / inputData.atmosColor[1], 4) * inputData.atmosScattering, 
            powf(400. / inputData.atmosColor[2], 4) * inputData.atmosScattering);

        glUniform1f(glGetUniformLocation(program, "mountainAmplitude"), inputData.mountainAmplitude);
        glUniform1f(glGetUniformLocation(program, "mountainFrequency"), inputData.mountainFrequency);

        glUniform1f(glGetUniformLocation(program, "seaLevel"), inputData.seaLevel);
        glUniform4f(glGetUniformLocation(program, "waterColor"), inputData.waterColor[0], inputData.waterColor[1], inputData.waterColor[2], inputData.waterColor[3]);
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

        std::vector<PlanetData> pdv{};
        PlanetData p1 = { .p = vec3(inputData.planetPos[0], inputData.planetPos[1], inputData.planetPos[2]), .radius = inputData.planetRadius, .mass = inputData.planetMass };
        pdv.push_back(p1);
        camera->setSpeedRef(inputData.cameraSpeed);
        camera->setJumpStrength(inputData.jumpStrength);
        camera->setMountainParams(inputData.mountainAmplitude, inputData.seaLevel);
        camera->update(dt, realTime, pdv);

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

        // Main render pass here
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