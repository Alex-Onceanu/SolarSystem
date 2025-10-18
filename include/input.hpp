#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

struct InputData {
    float sunPos[3];
    float sunColor[3];
    float sunCoronaStrength;

    float planetPos[3];
    float fov;

    float nb_steps_i;
    float nb_steps_j;
    float atmosRadius;
    float atmosFalloff;
    float atmosScattering;
    float atmosColor[3];
    float mountainFrequency;
    float mountainAmplitude;

    float seaLevel;
    float waterColor[4];
    float refractionindex;
    float fresnel;

    float ambientCoef;
    float diffuseCoef;
    float minDiffuse;
    float penumbraCoef;


    float nbStars;
    float starsDisplacement;
    float starSize;
    float starSizeVariation;
    float starVoidThreshold;
    float starFlickering;
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