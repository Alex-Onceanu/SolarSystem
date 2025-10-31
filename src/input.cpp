#include "input.hpp"
#include "init.h" // RESOLUTION_W

#if 0
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#endif

#include <string>

void Input::init(GLFWwindow* const window)
{
#if 0
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
#endif
}

void Input::destroy()
{
#if 0
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
#endif
}

InputData Input::getInput()
{
    static bool collapsed = true;

#if 0
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const int w = 480, h = 700;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::Begin("Bidouiller des constantes", &collapsed, ImGuiWindowFlags_NoResize);
#endif

    static InputData data{  .sunPos{ 0.,30.,10360. }, .sunRadius = 1242., .sunColor{ 1.0,1.0,0.5 }, .sunCoronaStrength = 9448.4,
                            .fov = 60., .cameraSpeed = 230., .jumpStrength = 450.,
                            .nb_steps_i = 9.01, .nb_steps_j = 6.01,
                            .atmosScattering = 0.2, .mountainFrequency = 8.,
                            .refractionindex = 0.75, .fresnel = 2.,
                            .ambientCoef = 0.02, .diffuseCoef = 0.21, .minDiffuse = 0.36, .penumbraCoef = 0.06,
                            .nbStars = 20000., .starsDisplacement = 0.069, .starSize = 2000., .starSizeVariation = 300., .starVoidThreshold = 0.249, .starFlickering = 1073. };
#if 0
    // data.planetPos[i][0] = 1400.; data.planetPos[i][1] = 1580.; data.planetPos[i][2] = 3300.;
    // data.planetRadius[i] = 500.;
    // data.planetMass[i] = 700000000.;
    // data.mountainAmplitude[i] = 80.;
    // data.atmosRadius[i] = 144.;
    // data.atmosFalloff[i] = 18.;
    // data.atmosColor[i][0] = 700.; data.atmosColor[i][1] = 530.; data.atmosColor[i][2] = 440.;
    auto io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f", io.Framerate);

    if (ImGui::CollapsingHeader("General"))
    {
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        ImGui::SliderFloat("fov", &data.fov, 10.0, 120.0);
        ImGui::SliderFloat("camera speed", &data.cameraSpeed, 10.0, 1000.0);
        ImGui::SliderFloat("jump strength", &data.jumpStrength, 1.0, 1000.0);
    }

    if (ImGui::CollapsingHeader("Sun"))
    {
        ImGui::SliderFloat3("sun position", data.sunPos, -40000., 40000.);
        ImGui::SliderFloat("sun radius", &data.sunRadius, 1.0, 2000.0);
        ImGui::ColorEdit3("sun color", data.sunColor);
        ImGui::SliderFloat("sun corona strength", &data.sunCoronaStrength, 1.0, 24000.0);
    }

    if (ImGui::CollapsingHeader("Water"))
    {
        ImGui::SliderFloat("refractionindex", &data.refractionindex, 0.1, 3.);
        ImGui::SliderFloat("fresnel", &data.fresnel, 0.1, 5.);
    }

    if (ImGui::CollapsingHeader("Light"))
    {
        ImGui::SliderFloat("ambientCoef", &data.ambientCoef, 0., 1.);
        ImGui::SliderFloat("diffuseCoef", &data.diffuseCoef, 0., 4.);
        ImGui::SliderFloat("minDiffuse", &data.minDiffuse, 0., 4.);
        ImGui::SliderFloat("penumbraCoef", &data.penumbraCoef, 0., 1.);
    }

    if (ImGui::CollapsingHeader("Atmosphere"))
    {
        ImGui::SliderFloat("atmos steps i", &data.nb_steps_i, 0., 30.);
        ImGui::SliderFloat("atmos steps j", &data.nb_steps_j, 0., 30.);
        ImGui::SliderFloat("atmos scattering", &data.atmosScattering, 0., 10.);
    }
    
    if (ImGui::CollapsingHeader("Background"))
    {
        ImGui::SliderFloat("nbStars", &data.nbStars, 100., 40000.);
        ImGui::SliderFloat("starsDisplacement", &data.starsDisplacement, 0., 0.1);
        ImGui::SliderFloat("starSize", &data.starSize, 500., 10000.);
        ImGui::SliderFloat("starSizeVariation", &data.starSizeVariation, 0., 1000.);
        ImGui::SliderFloat("starVoidThreshold", &data.starVoidThreshold, 0., 1.);
        ImGui::SliderFloat("starFlickering", &data.starFlickering, 0., 2000.);
    }

    // for(int i = 0; i < NB_PLANETS; i++)
    // {
    //     if (ImGui::CollapsingHeader((std::string("Planet") + std::to_string(i)).c_str()))
    //     {
    //         ImGui::SliderFloat3((std::string("Position") + std::to_string(i)).c_str(), data.planetPos[i], -40000., 40000.);
    //         ImGui::SliderFloat ((std::string("Radius")+ std::to_string(i)).c_str(), &(data.planetRadius[i]), 1., 2000.);
    //         ImGui::SliderFloat ((std::string("Mass")+ std::to_string(i)).c_str(), &(data.planetMass[i]), 1., 2000000000.);
    //         ImGui::SliderFloat ((std::string("Mtn height")+ std::to_string(i)).c_str(), &(data.mountainAmplitude[i]), .01, 150.);
    //         ImGui::SliderFloat ((std::string("atmos radius")+ std::to_string(i)).c_str(), &(data.atmosRadius[i]), 1., 800.0);
    //         ImGui::SliderFloat ((std::string("atmos falloff")+ std::to_string(i)).c_str(), &(data.atmosFalloff[i]), 0.0001, 40.0);
    //         ImGui::SliderFloat3((std::string("atmos lambda (nm)")+ std::to_string(i)).c_str(), data.atmosColor[i], 400., 800.);
    //         ImGui::SliderFloat ((std::string("sea level") + std::to_string(i)).c_str(), &(data.seaLevel[i]), 0., 1.);
    //         ImGui::ColorEdit4((std::string("water color") + std::to_string(i)).c_str(), data.waterColor[i]);
    //         ImGui::ColorEdit3((std::string("beach color") + std::to_string(i)).c_str(), data.beachColor[i]);
    //         ImGui::ColorEdit3((std::string("grass color") + std::to_string(i)).c_str(), data.grassColor[i]);
    //         ImGui::ColorEdit3((std::string("peak color") + std::to_string(i)).c_str(), data.peakColor[i]);
    //     }
    // }

    ImGui::End();
#endif
    return data;
}

void Input::renderInterface()
{
#if 0
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}