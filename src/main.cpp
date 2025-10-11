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

    Input::init(window);
    auto camera = std::make_unique<Camera>(window);

    auto startTime = std::chrono::high_resolution_clock::now();
    auto prevTime = startTime;
    auto lastSecondTime = startTime;
    unsigned int nbFramesThisSecond = 0;

    glUniform1f(glGetUniformLocation(program, "aspectRatio"), static_cast<float>(RESOLUTION_W) / static_cast<float>(RESOLUTION_H));

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
            // std::cout << "FPS : " << nbFramesThisSecond << std::endl;
            nbFramesThisSecond = 0;
            lastSecondTime = currentTime;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        unsigned int u_Texture = glGetUniformLocation(program, "earthTexture");
        glUniform1i(u_Texture, 0);

        auto inputData = Input::getInput();

        glUniform3f(glGetUniformLocation(program, "sunPos"), 
            inputData.sunPos[0], inputData.sunPos[1], inputData.sunPos[2]);
        glUniform3f(glGetUniformLocation(program, "sunColor"), 
            inputData.sunColor[0], inputData.sunColor[1], inputData.sunColor[2]);
        glUniform1f(glGetUniformLocation(program, "sunCoronaStrength"), 
            inputData.sunCoronaStrength);
        glUniform3f(glGetUniformLocation(program, "planetPos"), 
            inputData.planetPos[0], inputData.planetPos[1], inputData.planetPos[2]);
        glUniform3f(glGetUniformLocation(program, "planetColor"), 
            inputData.planetColor[0], inputData.planetColor[1], inputData.planetColor[2]);
        glUniform1f(glGetUniformLocation(program, "fov"), inputData.fov * 3.1415 / 180.);

        camera->update(dt);

        vec3 camPos = camera->getPos();
        glUniform3f(glGetUniformLocation(program ,"cameraPos"), camPos.x, camPos.y, camPos.z);

        vec2 camTheta = camera->getAngle();
        glUniform2f(glGetUniformLocation(program ,"cameraRotation"), camTheta.x, camTheta.y);

        // Draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        Input::renderInterface();

        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    Input::destroy();

    return 0;
}