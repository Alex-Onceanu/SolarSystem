#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

constexpr size_t NB_PLANETS = 8; // must be static because of uniform arrays in main.frag
struct InputData {
    float sunPos[3];
    float sunRadius;
    float sunColor[3];
    float sunCoronaStrength;

    float planetPos[NB_PLANETS][3];
    float planetRadius[NB_PLANETS];
    float planetMass[NB_PLANETS];

    float fov;
    float cameraSpeed;
    float jumpStrength;

    float nb_steps_i;
    float nb_steps_j;
    float atmosRadius[NB_PLANETS];
    float atmosFalloff[NB_PLANETS];
    float atmosScattering;
    float atmosColor[NB_PLANETS][3];
    float mountainFrequency;
    float mountainAmplitude[NB_PLANETS];
    float beachColor[NB_PLANETS][3];
    float grassColor[NB_PLANETS][3];
    float peakColor[NB_PLANETS][3];

    float seaLevel[NB_PLANETS];
    float waterColor[NB_PLANETS][4];
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