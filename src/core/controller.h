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

public:
    bool init();
    void unit();
    bool drawInput(float vMaxWidth);
    void drawPoints();
private:
    bool m_evalExpr(const std::string& vExpr);
    static P2dArray m_computePointsFromNumDigits(double vNum);
    static P2dArray m_computePointsFromNumDigits(const IntArray& vDigits);
    static IntArray m_getDecimalPartAsIntArray(double vNum);

public:  // singleton
    static Controller* Instance() {
        static Controller _instance;
        return &_instance;
    }
};
