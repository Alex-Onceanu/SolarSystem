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

    const int w = 240, h = 160;
    ImGui::SetNextWindowPos(ImVec2(RESOLUTION_W - w, 0));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::Begin("Bidouiller des constantes", &collapsed, ImGuiWindowFlags_NoResize);

    static InputData data{  .sunPos{ 0.,0.,8. }, .sunColor{ 1.0,1.0,0.5 }, .sunCoronaStrength = 2.0,
                            .planetPos{ 0.,-3.,27. }, .planetColor{ 0.3,0.2,1. }, .fov = 60. };

    ImGui::SliderFloat("fov", &data.fov, 10.0, 120.0);

    ImGui::InputFloat3("sun position", data.sunPos);
    ImGui::ColorEdit3("sun color", data.sunColor);
    ImGui::SliderFloat("sun corona strength", &data.sunCoronaStrength, 1.0, 8.0);
    
    ImGui::InputFloat3("planet position", data.planetPos);
    ImGui::ColorEdit3("planet color", data.planetColor);

    ImGui::End();
    return data;
}

void Input::renderInterface()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}