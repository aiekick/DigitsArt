#include "controller.h"
#include <cmath>
#include <ostream>
#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezExpr.hpp>

bool Controller::init() {
    return m_evalExpr("1/7");
}

void Controller::unit() {

}

bool Controller::drawInput(float vMaxWidth) {
    if (m_expr.DisplayInputText(vMaxWidth, "", "1/7")) {
        return m_evalExpr(m_expr.GetText());
    }
    return false;
}

void Controller::drawPoints() {
    auto *draw_list_ptr = ImGui::GetWindowDrawList();
    if (draw_list_ptr != nullptr) {
        ImVec2 lpt;
        auto colU32 = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
        for (size_t idx = 0; idx < m_points.size(); ++idx) {
            const auto &pt = m_points.at(idx);
            if (idx) {
                draw_list_ptr->AddLine(lpt, pt, colU32);
            }
            draw_list_ptr->AddCircleFilled(pt, 2.0f, colU32, 5);
            lpt = pt;
        }
    }
}

bool Controller::m_evalExpr(const std::string &vExpr) {
    try {
        ez::Expr ev;
        auto num = ev.parse(vExpr).eval().getResult();
        m_points = m_computePointsFromNumDigits(num);
        return true;
    } catch (const std::exception &ex) {
        //  LogVarError("ezExpr :parsing error : %s", ex.what());
    }
    return false;
}

P2dArray Controller::m_computePointsFromNumDigits(double vNum) {
    auto int_arr = m_getDecimalPartAsIntArray(vNum);
    return m_computePointsFromNumDigits(int_arr);
}

P2dArray Controller::m_computePointsFromNumDigits(const IntArray &vDigits) {
    P2dArray ret;
    float conf_step = 10.0f;
    float conf_angle = 9.0f / (2.0f * M_PI);
    ImVec2 pt;
    ret.push_back(pt);
    for (const auto& digit : vDigits){
        const auto ang = static_cast<float>(digit) * conf_angle;
        const auto dir = ez::fvec2 (cos(ang), sin(ang)).GetNormalized();
        pt += dir * conf_step;
        ret.push_back(pt);
    }
    return ret;
}

IntArray Controller::m_getDecimalPartAsIntArray(double vNum) {
    IntArray ret;
    if (vNum != 0.0) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(100) << vNum;
        std::string str = oss.str();
        size_t start = str.find('.');
        if (start != std::string::npos) {
            ++start;// +1 for jump the . of 0.
            for (size_t i = start; i < str.size(); ++i) {
                if (std::isdigit(str[i])) {
                    ret.push_back(str[i] - '0');
                }
            }
        }
    }
    return ret;
}
