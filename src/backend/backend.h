#pragma once

#include <imguipack.h>

#include <ezlibs/ezApp.hpp>
#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezXmlConfig.hpp>

#include <string>
#include <memory>
#include <array>
#include <vector>
#include <functional>
#include <unordered_map>

struct GLFWwindow;
class Backend : public ez::xml::Config {
private:
    GLFWwindow* m_MainWindowPtr = nullptr;
    const char* m_GlslVersion = "";
    ez::ivec2 m_DisplayPos;
    ez::ivec2 m_DisplaySize;

    // mouse
    ez::fvec4 m_MouseFrameSize;
    ez::fvec2 m_MousePos;
    ez::fvec2 m_LastNormalizedMousePos;
    ez::fvec2 m_NormalizedMousePos;

    bool m_ConsoleVisiblity = false;
    uint32_t m_CurrentFrame = 0U;

    bool m_NeedToCloseApp = false;  // when app closing app is required

    bool m_NeedToNewProject = false;
    bool m_NeedToLoadProject = false;
    bool m_NeedToCloseProject = false;
    std::string m_ProjectFileToLoad;

    std::function<void(std::set<std::string>)> m_ChangeFunc;
    std::set<std::string> m_PathsToTrack;

public:  // getters
    ImVec2 GetDisplayPos() {
        return ImVec2((float)m_DisplayPos.x, (float)m_DisplayPos.y);
    }
    ImVec2 GetDisplaySize() {
        return ImVec2((float)m_DisplaySize.x, (float)m_DisplaySize.y);
    }

public:
    virtual ~Backend();

    void run(const ez::App& vApp);

    bool init(const ez::App& vApp);
    void unit();

    bool isThereAnError() const;

    void PostRenderingActions();

    void setAppTitle(const std::string& vLabel);

    ez::dvec2 GetMousePos();
    int GetMouseButton(int vButton);

public:  // configuration
    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;

    void SetConsoleVisibility(const bool& vFlag);
    void SwitchConsoleVisibility();
    bool GetConsoleVisibility();

private:
    void m_RenderOffScreen();

    bool m_InitWindow();
    bool m_InitImGui();
    bool m_InitSystems();

    void m_UnitWindow();
    void m_UnitImGui();
    void m_UnitSystems();

    void m_MainLoop();
    void m_Update();
    void m_IncFrame();

public:  // singleton
    static Backend* Instance() {
        static Backend _instance;
        return &_instance;
    }
};
