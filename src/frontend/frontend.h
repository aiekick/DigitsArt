/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <imguipack.h>

#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezXmlConfig.hpp>

#include <backend/backend.h>

#include <functional>
#include <string>
#include <vector>
#include <map>

class Frontend : public ez::xml::Config {
private:
    bool m_ShowImGui = false;
    bool m_ShowMetric = false;
    ImFont* m_ToolbarFontPtr = nullptr;
    ImRect m_DisplayRect = ImRect(ImVec2(0, 0), ImVec2(1280, 720));
    bool m_ShowAboutDialog = false;          // show about dlg
    bool m_SaveDialogIfRequired = false;     // open save options dialog (save / save as / continue without saving / cancel)
    bool m_SaveDialogActionWasDone = false;  // if action was done by save options dialog
    ImCanvas m_canvas;
    bool m_firstDraw = true;
    bool m_drawGrid = true;
    bool m_drawScales = true;
    std::vector<ImVec2> m_points;

public:
    static bool sCentralWindowHovered;

public:
    virtual ~Frontend();

    bool init();
    void unit();

    bool isValid() const;
    bool isThereAnError() const;

    void Display(const uint32_t& vCurrentFrame, const ImRect& vRect);

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas);
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas);
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect,  ImGuiContext* vContextPtr, void* vUserDatas);

  // configuration
    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;

private:
    bool m_build();
    bool m_build_themes();
    void m_drawMainMenuBar();
    void m_drawMainStatusBar();
    void m_drawExpr(const ImRect& vRect);
    void m_drawCanvas(const ImRect& vRect);

public:  // singleton
    static Frontend* Instance() {
        static Frontend _instance;
        return &_instance;
    };
};
