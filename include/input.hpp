#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

struct InputData {
    float sunPos[3];
    float sunColor[3];
    float sunCoronaStrength;

    float planetPos[3];
    float planetColor[3];
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