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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "frontend.h"

#include <imguipack.h>

#include <core/controller.h>

#include <res/fontIcons.h>

using namespace std::placeholders;

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool Frontend::sCentralWindowHovered = false;

//////////////////////////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

Frontend::~Frontend() = default;

bool Frontend::init() {
    m_build_themes();
    m_canvas.getConfigRef().zoomFactor = 0.1f;
    m_canvas.getConfigRef().zoomStep = 0.1f;
    m_canvas.getConfigRef().draggingButton = ImGuiMouseButton_Left;
    return m_build();
}

void Frontend::unit() {
}

bool Frontend::isValid() const {
    return false;
}

bool Frontend::isThereAnError() const {
    return false;
}

void Frontend::Display(const uint32_t &vCurrentFrame, const ImRect &vRect) {
    const auto context_ptr = ImGui::GetCurrentContext();
    if (context_ptr != nullptr) {
        const auto &io = ImGui::GetIO();

        m_DisplayRect = vRect;

        Frontend::sCentralWindowHovered = (ImGui::GetCurrentContext()->HoveredWindow == nullptr);
        ImGui::CustomStyle::ResetCustomId();

        m_drawMainMenuBar();
        m_drawCanvas();
        m_drawMainStatusBar();

        DrawDialogsAndPopups(vCurrentFrame, m_DisplayRect, context_ptr, {});

        ImGuiThemeHelper::Instance()->Draw();
    }
}

bool Frontend::DrawWidgets(const uint32_t &vCurrentFrame, ImGuiContext *vContextPtr, void *vUserDatas) {
    bool res = false;
    return res;
}

bool Frontend::DrawOverlays(const uint32_t &vCurrentFrame, const ImRect &vRect, ImGuiContext *vContextPtr,
                            void *vUserDatas) {
    bool res = false;
    return res;
}

bool Frontend::DrawDialogsAndPopups(
        const uint32_t &vCurrentFrame, const ImRect &vRect, ImGuiContext *vContextPtr, void *vUserDatas) {
    if (m_ShowImGui) {
        ImGui::ShowDemoWindow(&m_ShowImGui);
    }
    if (m_ShowMetric) {
        ImGui::ShowMetricsWindow(&m_ShowMetric);
    }
    return false;
}

void Frontend::m_drawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(ICON_FONT_TEXTURE " Canvas")) {
            if (ImGui::MenuItem("Reset")) {
                m_firstDraw = true;
            }
            if (ImGui::BeginMenu("Draw")) {
                ImGui::MenuItem("grid", nullptr, &m_drawGrid);
                ImGui::MenuItem("scales", nullptr, &m_drawScales);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_FONT_SETTINGS " Tools")) {
            if (ImGui::BeginMenu("Styles")) {
                ImGuiThemeHelper::Instance()->DrawMenu();

                if (m_drawGrid || m_drawScales) {
                    if (ImGui::BeginMenu("Canvas")) {
                        ImGui::SliderFloatDefaultCompact(250.0f, "Major step X", &m_canvas.getConfigRef().gridSize.x,
                                                         1.0f, 200.0f, 50.0f, 1.0f);
                        ImGui::SliderFloatDefaultCompact(250.0f, "Major step Y", &m_canvas.getConfigRef().gridSize.y,
                                                         1.0f, 200.0f, 50.0f, 1.0f);
                        ImGui::SliderFloatDefaultCompact(250.0f, "Subdivs X", &m_canvas.getConfigRef().gridSubdivs.x,
                                                         0.0f, 50.0f, 5.0f, 1.0f);
                        ImGui::SliderFloatDefaultCompact(250.0f, "Subdivs Y", &m_canvas.getConfigRef().gridSubdivs.y,
                                                         0.0f, 50.0f, 5.0f, 1.0f);
                        ImGui::EndMenu();
                    }
                }

                ImGui::Separator();
                ImGui::MenuItem("Show ImGui", "", &m_ShowImGui);
                ImGui::MenuItem("Show ImGui Metric/Debug", "", &m_ShowMetric);

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        const auto label = ez::str::toStr("Dear ImGui %s (Docking)", ImGui::GetVersion());
        const auto size = ImGui::CalcTextSize(label.c_str());

        Controller::Instance()->drawInput(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);

        ImGui::Text("%s", label.c_str());

        ImGui::EndMainMenuBar();
    }
}

void Frontend::m_drawMainStatusBar() {
    if (ImGui::BeginMainStatusBar()) {
        Messaging::Instance()->DrawStatusBar();

#ifdef _DEBUG
        const auto &io = ImGui::GetIO();
        const auto fps = ez::str::toStr("%.1f ms/frame (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);
        const auto size = ImGui::CalcTextSize(fps.c_str());
        ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::Text("%s", fps.c_str());
#endif

        ImGui::EndMainStatusBar();
    }
}

void Frontend::m_drawCanvas() {
    const auto *viewport_ptr = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport_ptr->WorkPos);
    ImGui::SetNextWindowSize(viewport_ptr->WorkSize);
    ImGui::SetNextWindowViewport(viewport_ptr->ID);
    if (ImGui::Begin("CanvasWindow", nullptr, ImGuiWindowFlags_NoTitleBar)) {
        const auto content_size = ImGui::GetContentRegionAvail();
        if (m_canvas.begin("Canvas", content_size)) {
            if (m_drawGrid) {
                m_canvas.drawGrid();
            }
            if (m_drawScales) {
                m_canvas.drawScales();
            }
            if (m_firstDraw) {
                m_canvas.getViewRef().set(content_size * 0.5f, 1.0f);
                m_firstDraw = false;
            }
            Controller::Instance()->drawPoints();
            m_canvas.end();
        }
    }
    ImGui::End();
}

//////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool Frontend::m_build() {
    // toolbar
    /*static ImFontConfig icons_config3;
    icons_config3.MergeMode = false;
    icons_config3.PixelSnapH = true;
    static const ImWchar icons_ranges3[] = {ICON_MIN_SDFMT, ICON_MAX_SDFMT, 0};
    const float& font_size = 20.0f / font_scale_ratio;
    m_ToolbarFontPtr =
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_SDFMT, font_size, &icons_config3, icons_ranges3);
    if (m_ToolbarFontPtr != nullptr) {
        m_ToolbarFontPtr->Scale = font_scale_ratio;
        return true;
    }*/

    return true;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

ez::xml::Nodes Frontend::getXmlNodes(const std::string &vUserDatas) {
    ez::xml::Node node;
    node.addChilds(ImGuiThemeHelper::Instance()->getXmlNodes("app"));
#ifdef USE_PLACES_FEATURE
    node.addChild("places").setContent(ImGuiFileDialog::Instance()->SerializePlaces());
#endif
    node.addChild("showimgui").setContent(m_ShowImGui);
    node.addChild("showmetric").setContent(m_ShowMetric);
    return node.getChildren();
}

bool
Frontend::setFromXmlNodes(const ez::xml::Node &vNode, const ez::xml::Node &vParent, const std::string &vUserDatas) {
    const auto &strName = vNode.getName();
    const auto &strValue = vNode.getContent();
    const auto &strParentName = vParent.getName();

    if (strName == "places") {
#ifdef USE_PLACES_FEATURE
        ImGuiFileDialog::Instance()->DeserializePlaces(strValue);
#endif
    } else if (strName == "showimgui") {
        m_ShowImGui = ez::ivariant(strValue).GetB();
    } else if (strName == "showmetric") {
        m_ShowMetric = ez::ivariant(strValue).GetB();
    }

    ImGuiThemeHelper::Instance()->setFromXmlNodes(vNode, vParent, "app");

    return true;
}
