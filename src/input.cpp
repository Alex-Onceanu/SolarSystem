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

    const int w = 360, h = 460;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::Begin("Bidouiller des constantes", &collapsed, ImGuiWindowFlags_NoResize);

    static InputData data{  .sunPos{ -10.,30.,260. }, .sunColor{ 1.0,1.0,0.5 }, .sunCoronaStrength = 30.0,
                            .planetPos{ 0.,-60.,50. }, .planetColor{ 0.3,0.2,1. }, .fov = 60.,
                            .nb_steps_i = 14., .nb_steps_j = 6.,
                            .atmosRadius = 18., .atmosFalloff = 18., .atmosScattering = 1.,
                            .atmosColor{700., 530., 440.} };

    ImGui::SliderFloat("fov", &data.fov, 10.0, 120.0);

    ImGui::SliderFloat3("sun position", data.sunPos, -500., 500.);
    ImGui::ColorEdit3("sun color", data.sunColor);
    ImGui::SliderFloat("sun corona strength", &data.sunCoronaStrength, 1.0, 30.0);
    
    ImGui::SliderFloat3("planet position", data.planetPos, -500., 500.);
    ImGui::ColorEdit3("planet color", data.planetColor);
    
    ImGui::SliderFloat("atmos steps i", &data.nb_steps_i, 0., 30.);
    ImGui::SliderFloat("atmos steps j", &data.nb_steps_j, 0., 30.);
    ImGui::SliderFloat("atmos radius", &data.atmosRadius, 0., 50.0);
    ImGui::SliderFloat("atmos falloff", &data.atmosFalloff, 1., 50.0);
    ImGui::SliderFloat("atmos scattering", &data.atmosScattering, 0., 10.);
    ImGui::SliderFloat3("atmos λ (nm)", data.atmosColor, 400., 800.);

    ImGui::End();
    return data;
}

void Input::renderInterface()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}