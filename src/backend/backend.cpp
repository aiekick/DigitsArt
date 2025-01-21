#include "backend.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <headers/DigitsArtBuild.h>

#define IMGUI_IMPL_API
#include <3rdparty/imgui_docking/backends/imgui_impl_opengl3.h>
#include <3rdparty/imgui_docking/backends/imgui_impl_glfw.h>

#include <cstdio>     // printf, fprintf
#include <chrono>     // timer
#include <cstdlib>    // abort
#include <fstream>    // std::ifstream
#include <iostream>   // std::cout
#include <algorithm>  // std::min, std::max
#include <stdexcept>  // std::exception

#include <imguipack.h>

#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezFile.hpp>
#include <frontend/frontend.h>

#include <core/controller.h>

// we include the cpp just for embedded fonts
#include <res/fontIcons.cpp>
#include <res/robotoMedium.cpp>

#define INITIAL_WIDTH 1700
#define INITIAL_HEIGHT 700

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

static void glfw_error_callback(int error, const char* description) {
    LogVarError("glfw error %i : %s", error, description);
}

static void glfw_drop_callback(GLFWwindow* window, int path_count, const char* paths[]) {
    std::vector<std::string> files;
    files.reserve(path_count);
    for (int idx = 0; idx < path_count; ++idx) {
        files.push_back(paths[idx]);
    }
    EZ_TOOLS_DEBUG_BREAK;
}

//////////////////////////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

Backend::~Backend() = default;

void Backend::run(const ez::App& vApp) {
    if (init(vApp)) {
        m_MainLoop();
        unit();
    }
}

// todo : to refactor ! i dont like that
bool Backend::init(const ez::App& vApp) {
#ifdef _DEBUG
    SetConsoleVisibility(true);
#else
    SetConsoleVisibility(false);
#endif
    if (m_InitWindow()) {
        if (m_InitImGui()) {
            if (m_InitSystems()) {
                LoadConfigFile("config.xml", "app");
                setAppTitle({});
                return true;
            }
        }
    }
    return false;
}

// todo : to refactor ! i dont like that
void Backend::unit() {
    SaveConfigFile("config.xml", "app", "config");
    m_UnitSystems();
    m_UnitImGui();
    m_UnitWindow();
}

bool Backend::isThereAnError() const {
    return false;
}

// actions to do after rendering
void Backend::PostRenderingActions() {

}

void Backend::setAppTitle(const std::string& vLabel) {
    if (!vLabel.empty()) {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "DigitArt Beta %s - %s.lum", DigitsArt_BuildId, vLabel.c_str());
        glfwSetWindowTitle(m_MainWindowPtr, bufTitle);
    } else {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "DigitArt Beta %s", DigitsArt_BuildId);
        glfwSetWindowTitle(m_MainWindowPtr, bufTitle);
    }
}

ez::dvec2 Backend::GetMousePos() {
    ez::dvec2 mp;
    glfwGetCursorPos(m_MainWindowPtr, &mp.x, &mp.y);
    return mp;
}

int Backend::GetMouseButton(int vButton) {
    return glfwGetMouseButton(m_MainWindowPtr, vButton);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONSOLE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Backend::SetConsoleVisibility(const bool& vFlag) {
    m_ConsoleVisiblity = vFlag;

    if (m_ConsoleVisiblity) {
        // on cache la console
        // on l'affichera au besoin comme blender fait
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
    } else {
        // on cache la console
        // on l'affichera au besoin comme blender fait
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
    }
}

void Backend::SwitchConsoleVisibility() {
    m_ConsoleVisiblity = !m_ConsoleVisiblity;
    SetConsoleVisibility(m_ConsoleVisiblity);
}

bool Backend::GetConsoleVisibility() {
    return m_ConsoleVisiblity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// RENDER ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Backend::m_RenderOffScreen() {
    // m_DisplaySizeQuadRendererPtr->SetImageInfos(m_MergerRendererPtr->GetBackDescriptorImageInfo(0U));
}

void Backend::m_MainLoop() {
    int display_w, display_h;
    ImRect viewRect;
    while (!glfwWindowShouldClose(m_MainWindowPtr)) {
        // maintain active, prevent user change via imgui dialog
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Disable Viewport

        glfwPollEvents();

        glfwGetFramebufferSize(m_MainWindowPtr, &display_w, &display_h);

        m_Update();  // to do absolutly before imgui rendering

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport) {
            viewRect.Min = viewport->WorkPos;
            viewRect.Max = viewRect.Min + viewport->WorkSize;
        } else {
            viewRect.Max = ImVec2((float)display_w, (float)display_h);
        }

        Frontend::Instance()->Display(m_CurrentFrame, viewRect);

        ImGui::Render();

        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        auto* backup_current_context = glfwGetCurrentContext();

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste
        // this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(m_MainWindowPtr);

        // mainframe post actions
        if (m_CurrentFrame > 0) {
            // > 1 is needed for the graph offset and scale can be applied
            PostRenderingActions();
        } 

        ++m_CurrentFrame;

        // will pause the view until we move the mouse or press keys
        // glfwWaitEvents();
    }
}

void Backend::m_Update() {

}

void Backend::m_IncFrame() {
    ++m_CurrentFrame;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

ez::xml::Nodes Backend::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    node.addChilds(Frontend::Instance()->getXmlNodes(vUserDatas));
    return node.getChildren();
}

bool Backend::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    const auto& strName = vNode.getName();
    const auto& strValue = vNode.getContent();
    const auto& strParentName = vParent.getName();
    Frontend::Instance()->setFromXmlNodes(vNode, vParent, vUserDatas);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool Backend::m_InitWindow() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return false;
    }

    // GL 3.0 + GLSL 130
    m_GlslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window with graphics context
    m_MainWindowPtr = glfwCreateWindow(1280, 720, "DigitsArt", nullptr, nullptr);
    if (m_MainWindowPtr == nullptr) {
        std::cout << "Fail to create the window" << std::endl;
        return false;
    }
    glfwMakeContextCurrent(m_MainWindowPtr);
    glfwSwapInterval(1);  // Enable vsync

    if (gladLoadGL() == 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return false;
    }

    glfwSetDropCallback(m_MainWindowPtr, glfw_drop_callback);

    return true;
}

void Backend::m_UnitWindow() {
    glfwDestroyWindow(m_MainWindowPtr);
    glfwTerminate();
}

bool Backend::m_InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Viewport
    //io.FontAllowUserScaling = true;                      // activate zoom feature with ctrl + mousewheel
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
    io.ConfigViewportsNoDecoration = false;  // toujours mettre une frame aux fenetres enfants
#endif
    
    float dpiScaleFactor = 100.0f / 16.0f;

    // fonts
    {
        {  // main font
            auto fontPtr = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_RM, 16.0f * dpiScaleFactor);
            if (fontPtr == nullptr) {
                assert(0);  // failed to load font
            } else {
                fontPtr->Scale = 1.0f / dpiScaleFactor;
            }
        }
        {  // icon font
            static const ImWchar icons_ranges[] = {ICON_MIN_FONT, ICON_MAX_FONT, 0};
            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            auto fontPtr = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_FONT, 16.0f * dpiScaleFactor, &icons_config, icons_ranges);
            if (fontPtr == nullptr) {
                assert(0);  // failed to load font
            } else {
                fontPtr->Scale = 1.0f / dpiScaleFactor;
            }
        }
    }    

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    style.ScaleAllSizes(dpiScaleFactor);
    
    // Setup Platform/Renderer bindings
    if (ImGui_ImplGlfw_InitForOpenGL(m_MainWindowPtr, true) &&  //
        ImGui_ImplOpenGL3_Init(m_GlslVersion)) {

        // ui init
        if (Frontend::Instance()->init()) {
            return true;
        }
    }
    return false;
}

bool Backend::m_InitSystems() {
    bool ret = true;
    ret &= Controller::Instance()->init();
    return ret;
}

void Backend::m_UnitSystems() {
    Controller::Instance()->unit();
}

void Backend::m_UnitImGui() {
    Frontend::Instance()->unit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
}

