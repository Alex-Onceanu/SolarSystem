#include "input.hpp"
#include "init.h" // RESOLUTION_W

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

void Input::init(GLFWwindow* const window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

void Input::destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

InputData Input::getInput()
{
    static bool collapsed = true;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const int w = 380, h = 400;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::Begin("Bidouiller des constantes", &collapsed, ImGuiWindowFlags_NoResize);

    static InputData data{  .sunPos{ -10.,30.,10360. }, .sunRadius = 500., .sunColor{ 1.0,1.0,0.5 }, .sunCoronaStrength = 1600.0,
                            .planetPos{ 0.,-280.,200. }, .planetRadius = 300., .planetMass = 1800000.,
                            .fov = 60., .cameraSpeed = 60.,
                            .nb_steps_i = 8.1, .nb_steps_j = 4.1,
                            .atmosRadius = 36., .atmosFalloff = 6.4, .atmosScattering = 1.4, .atmosColor{700., 530., 440.},
                            .mountainFrequency = 8., .mountainAmplitude = 30., 
                            .seaLevel = .6, .waterColor{ 0.,0.26,0.46,0.2 }, .refractionindex = 0.75, .fresnel = 2.,
                            .ambientCoef = 0.04, .diffuseCoef = 0.85, .minDiffuse = 0.22, .penumbraCoef = 0.07,
                            .nbStars = 20000., .starsDisplacement = 0.069, .starSize = 2000., .starSizeVariation = 300., .starVoidThreshold = 0.249, .starFlickering = 1073. };

    auto io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f", io.Framerate);

    if (ImGui::CollapsingHeader("General"))
    {
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        ImGui::SliderFloat("fov", &data.fov, 10.0, 120.0);
        ImGui::SliderFloat("camera speed", &data.cameraSpeed, 10.0, 10000.0);
    }

    if (ImGui::CollapsingHeader("Sun"))
    {
        ImGui::SliderFloat3("sun position", data.sunPos, -500000., 500000.);
        ImGui::SliderFloat("sun radius", &data.sunRadius, 1.0, 60000.0);
        ImGui::ColorEdit3("sun color", data.sunColor);
        ImGui::SliderFloat("sun corona strength", &data.sunCoronaStrength, 1.0, 240000.0);
    }

    if (ImGui::CollapsingHeader("Planet"))
    {
        ImGui::SliderFloat3("planet position", data.planetPos, -500000., 500000.);
        ImGui::SliderFloat("planet radius", &data.planetRadius, 1., 2000.);
        ImGui::SliderFloat("planet mass", &data.planetMass, 1., 10000000000.);
        ImGui::SliderFloat("mountain height", &data.mountainAmplitude, .01, 600.);
    }
    if (ImGui::CollapsingHeader("Water"))
    {
        ImGui::SliderFloat("sea level", &data.seaLevel, 0., 1.);
        ImGui::ColorEdit4("waterColor", data.waterColor);
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
        ImGui::SliderFloat("atmos radius", &data.atmosRadius, 0., 5000.0);
        ImGui::SliderFloat("atmos falloff", &data.atmosFalloff, 0.0001, 40.0);
        ImGui::SliderFloat("atmos scattering", &data.atmosScattering, 0., 10.);
        ImGui::SliderFloat3("atmos lambda (nm)", data.atmosColor, 400., 800.);
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

    ImGui::End();
    return data;
}

void Input::renderInterface()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}