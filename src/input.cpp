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

    const int w = 360, h = 560;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::Begin("Bidouiller des constantes", &collapsed, ImGuiWindowFlags_NoResize);

    static InputData data{  .sunPos{ -10.,30.,260. }, .sunColor{ 1.0,1.0,0.5 }, .sunCoronaStrength = 40.0,
                            .planetPos{ 0.,-60.,50. }, .planetColor{ 0.3,0.2,1. }, .fov = 60.,
                            .nb_steps_i = 8.1, .nb_steps_j = 4.1,
                            .atmosRadius = 18., .atmosFalloff = 6., .atmosScattering = 1.5, .atmosColor{700., 530., 440.},
                            .mountainFrequency = 8., .mountainAmplitude = 8., .seaLevel = .3,
                            .nbStars = 20000., .starsDisplacement = 0.069, .starSize = 2000., .starSizeVariation = 300., .starVoidThreshold = 0.249, .starFlickering = 873. };

    auto io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
    ImGui::SliderFloat("fov", &data.fov, 10.0, 120.0);

    ImGui::SliderFloat3("sun position", data.sunPos, -500., 500.);
    ImGui::ColorEdit3("sun color", data.sunColor);
    ImGui::SliderFloat("sun corona strength", &data.sunCoronaStrength, 1.0, 60.0);
    
    ImGui::SliderFloat3("planet position", data.planetPos, -500., 500.);
    
    ImGui::SliderFloat("atmos steps i", &data.nb_steps_i, 0., 30.);
    ImGui::SliderFloat("atmos steps j", &data.nb_steps_j, 0., 30.);
    ImGui::SliderFloat("atmos radius", &data.atmosRadius, 0., 50.0);
    ImGui::SliderFloat("atmos falloff", &data.atmosFalloff, 0.0001, 40.0);
    ImGui::SliderFloat("atmos scattering", &data.atmosScattering, 0., 10.);
    ImGui::SliderFloat3("atmos lambda (nm)", data.atmosColor, 400., 800.);

    ImGui::SliderFloat("mountain height", &data.mountainAmplitude, 0.01, 20.);
    ImGui::SliderFloat("sea level", &data.seaLevel, 0., 1.);

    ImGui::SliderFloat("nbStars", &data.nbStars, 100., 40000.);
    ImGui::SliderFloat("starsDisplacement", &data.starsDisplacement, 0., 0.1);
    ImGui::SliderFloat("starSize", &data.starSize, 500., 10000.);
    ImGui::SliderFloat("starSizeVariation", &data.starSizeVariation, 0., 1000.);
    ImGui::SliderFloat("starVoidThreshold", &data.starVoidThreshold, 0., 1.);
    ImGui::SliderFloat("starFlickering", &data.starFlickering, 0., 1000.);

    ImGui::End();
    return data;
}

void Input::renderInterface()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}