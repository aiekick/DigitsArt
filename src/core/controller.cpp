#include "controller.h"
#include <cmath>
#include <ostream>
#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezExpr.hpp>

#define NUM_STR "3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648566923460348610454326648213393607260249141273724587006606315588174881520920962829254091715364367892590360011330530548820466521384146951941511609433057270365759591953092186117381932611793105118548074462379962749567351885752724891227938183011949129833673362440656643086021394946395224737190702179860943702770539217176293176752384674818467669405132000568127145263560827785771342757789609173637178721468440901224953430146549585371050792279689258923542019956112129021960864034418159813629774771309960518707211349999998372978049951059731732816096318595024459455346908302642522308253344685035261931188171010003137838752886587533208381420617177669147303598253490428755468731159562863882353787593751957781857780532171226806613001927876611195909216420199"

bool Controller::init() {
    m_modes.init(0, {"Nombre", "Expr"});
    if (m_evalExpr(NUM_STR)) {
        m_countPoints = m_points.size();
        return true;
    }
    return false;
}

void Controller::unit() {

}

bool Controller::drawInput(float vMaxWidth) {
    const auto w = 120.0f;
    if (m_modes.display(w, "##Modes")) {
        return m_evalExpr(m_expr.GetText());
    }
    if (m_expr.DisplayInputText(vMaxWidth - w, "", NUM_STR)) {
        return m_evalExpr(m_expr.GetText());
    }
    return false;
}

bool Controller::drawControl(float vMaxWidth) {
    float w = (vMaxWidth - ImGui::GetStyle().ItemSpacing.x * 8.0f - ImGui::GetStyle().FramePadding.x * 2.0f) * 0.25f;
    bool change = false;
    change |= ImGui::SliderFloatDefaultCompact(w, "step size", &m_stepSize, 0.0f, 100.0f, 10.0f, 0.0f, "%.6f");
    change |= ImGui::SliderFloatDefaultCompact(w, "angle lim inf", &m_angleLimInf, 0.0f, 1.0f, 0.998f, 0.0f, "%.6f");
    change |= ImGui::SliderFloatDefaultCompact(w, "angle factor", &m_angleFactor, m_angleLimInf, 1.0f, 1.0f, 0.0f, "%.6f");
    change |= ImGui::SliderSizeTDefaultCompact(w, "count", &m_countPoints, 0U, m_points.size(), m_points.size());
    if (change) {
        m_evalExpr(m_expr.GetText());
    }
    return change;
}

void Controller::drawPoints() {
    auto *draw_list_ptr = ImGui::GetWindowDrawList();
    if (draw_list_ptr != nullptr) {
        ImVec2 lpt;
        auto colU32 = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
        size_t count = ez::mini(m_countPoints, m_points.size());
        for (size_t idx = 0; idx < count; ++idx) {
            const auto &pt = m_points.at(idx);
            if (idx) {
                draw_list_ptr->AddLine(lpt, pt, colU32);
            }
            draw_list_ptr->AddCircleFilled(pt, 1.2f, colU32, 5);
            lpt = pt;
        }
    }
}

bool Controller::m_evalExpr(const std::string &vExpr) {
    try {
        if (m_modes.getIndex() == 0) {
            m_points = m_computePointsFromNumDigits(vExpr);
        } else if (m_modes.getIndex() == 1) {
            ez::Expr ev;
            auto num = ev.parse(vExpr).eval().getResult();
            m_points = m_computePointsFromNumDigits(num);
        }
        return true;
    } catch (...) {
        m_points.clear();
    }
    return false;
}

P2dArray Controller::m_computePointsFromNumDigits(double vNum) {
    auto int_arr = m_getDecimalPartAsIntArray(vNum);
    return m_computePointsFromNumDigits(int_arr);
}

P2dArray Controller::m_computePointsFromNumDigits(const std::string &vNum) {
    auto int_arr = m_getDecimalPartAsIntArray(vNum);
    return m_computePointsFromNumDigits(int_arr);
}

P2dArray Controller::m_computePointsFromNumDigits(const IntArray &vDigits) {
    P2dArray ret;
    ImVec2 pt;
    float lan = 0.0f;
    ret.push_back(pt);
    for (const auto& digit : vDigits){
        const auto an = lan + sin(static_cast<float>(digit) * m_angleFactor) * M_2_PI;
        const auto dir = ez::fvec2(cos(an), sin(an)).GetNormalized();
        pt += dir * m_stepSize;
        ret.push_back(pt);
        lan = an;
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
            auto part = str.substr(start + 1);
            for (size_t idx = 0; idx < 10; ++idx) {
                str += part;
            }
        }
        ret = m_getDecimalPartAsIntArray(str);
    }
    return ret;
}

IntArray Controller::m_getDecimalPartAsIntArray(const std::string& vNum) {
    IntArray ret;
    if (!vNum.empty()) {
        size_t start = vNum.find('.');
        if (start != std::string::npos) {
            ++start;  // +1 for jump the . of 0.
            for (size_t i = start; i < vNum.size(); ++i) {
                if (std::isdigit(vNum[i])) {
                    ret.push_back(vNum[i] - '0');
                }
            }
        }
    }
    return ret;
}
