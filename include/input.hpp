#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

struct InputData {

};

class Input
{
public:
    static void init(GLFWwindow* const window);
    static void destroy();

    static InputData getInput();
    static void renderInterface();
};

#endif // INPUT_H
