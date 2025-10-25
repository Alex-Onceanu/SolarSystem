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

    auto earthTexture = init_texture("../assets/eart.ppm");
    auto opticalDepthTexture = init_texture("../assets/noise.pgm");

    Input::init(window);
    auto camera = std::make_unique<Camera>(window, vec3(0., 40.0, -50.0), vec3(0.,-280.,200.));

    auto startTime = std::chrono::high_resolution_clock::now();
    auto prevTime = startTime;
    auto lastSecondTime = startTime;
    unsigned int nbFramesThisSecond = 0;

    glUniform1f(glGetUniformLocation(program, "aspectRatio"), static_cast<float>(RESOLUTION_W) / static_cast<float>(RESOLUTION_H));

    unsigned int frameBuf, outTexture;
    generateLowResBuf(&frameBuf, &outTexture);

    // Otherwise we see a pink screen as very first frame when launching the program
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    // mainloop
    while (!glfwWindowShouldClose(window))
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float timeSinceLastSecond = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastSecondTime).count();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
        prevTime = currentTime;

        nbFramesThisSecond++;
        if(timeSinceLastSecond >= 1.0f)
        {
            nbFramesThisSecond = 0;
            lastSecondTime = currentTime;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        glUniform1i(glGetUniformLocation(program, "earthTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, opticalDepthTexture);
        glUniform1i(glGetUniformLocation(program, "opticalDepthTexture"), 1);

        auto inputData = Input::getInput();
        glUniform1f(glGetUniformLocation(program, "time"), elapsedTime);
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

        std::vector<PlanetData> pdv;
        PlanetData p1 = { .p = vec3(inputData.planetPos[0], inputData.planetPos[1], inputData.planetPos[2]), .radius = 30. + 300., .mass = inputData.planetMass };
        pdv.push_back(p1);
        camera->setSpeedRef(inputData.cameraSpeed);
        camera->update(dt, pdv);

        vec3 camPos = camera->getPos();
        glUniform3f(glGetUniformLocation(program ,"cameraPos"), camPos.x, camPos.y, camPos.z);

        float view[9];
        camera->getView(view);
        glUniformMatrix3fv(glGetUniformLocation(program ,"view"), 1, false, view);

        int W, H;
        glfwGetWindowSize(window, &W, &H);

        // Draw
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuf);
        glViewport(0, 0, LOW_RES_W, LOW_RES_H);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuf);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, LOW_RES_W, LOW_RES_H, 0, 0, W, H, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        Input::renderInterface();

        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    Input::destroy();

    return 0;
}