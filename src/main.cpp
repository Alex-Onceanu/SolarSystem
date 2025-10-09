#include <glad.h>

#include "init.h"
#include "input.hpp"

int main()
{
    GLFWwindow* window = nullptr;
    unsigned int program = init(&window);
    Input::init(window);

    // mainloop
    while (!glfwWindowShouldClose(window))
    {
        Input::getInput();

        // Clear
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        Input::renderInterface();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    Input::destroy();

    return 0;
}