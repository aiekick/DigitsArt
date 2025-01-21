#pragma once

#include <vector>
#include <cstdint>
#include <imguipack.h>

typedef std::vector<ImVec2> P2dArray;
typedef std::vector<int32_t> IntArray;

class Controller {
private:
    ImWidgets::InputText m_expr;
    std::vector<ImVec2> m_points;
    float m_stepSize = 10.0f;
    float m_angleLimInf = 0.998f;
    size_t m_countPoints = 100U;
    float m_angleFactor = 1.0f;
    ImWidgets::QuickStringCombo m_modes;

public:
    bool init();
    void unit();
    bool drawInput(float vMaxWidth);
    bool drawControl(float vMaxWidth);
    void drawPoints();

private:
    bool m_evalExpr(const std::string& vExpr);
    P2dArray m_computePointsFromNumDigits(double vNum);
    P2dArray m_computePointsFromNumDigits(const std::string& vNum);
    P2dArray m_computePointsFromNumDigits(const IntArray& vDigits);
    IntArray m_getDecimalPartAsIntArray(double vNum);
    IntArray m_getDecimalPartAsIntArray(const std::string& vNum);

public:  // singleton
    static Controller* Instance() {
        static Controller _instance;
        return &_instance;
    }
};
