// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "app.h"

#include <headers/DigitArtBuild.h>
#include <backend/backend.h>

#include <ezlibs/ezApp.hpp>
#include <ezlibs/ezFile.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezFigFont.hpp> 

#include <headers/customMessagingConfigHeader.h>

// messaging
#define MESSAGING_CODE_INFOS 0
#define MESSAGING_LABEL_INFOS  "Infos"
#define MESSAGING_CODE_WARNINGS 1
#define MESSAGING_LABEL_WARNINGS  "Warnings"
#define MESSAGING_CODE_ERRORS 2
#define MESSAGING_CODE_DEBUG 3
#define MESSAGING_LABEL_ERRORS  "Errors"
#define MESSAGING_LABEL_DEBUG  "Debug"

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int App::run(int argc, char** argv) {
    ez::App app(argc, argv);
    // https://patorjk.com/software/taag/#p=display&h=1&v=0&f=Big&t=DigitArt%20v0.1
    std::cout << DigitArt_FigFontLabel << std::endl;
    std::cout << "-----------" << std::endl;
    LogVarLightInfo("[[ %s Beta v%s ]]", DigitArt_Label, DigitArt_BuildId);
    Messaging::Instance()->AddCategory(MESSAGING_CODE_INFOS, "Info(s)", BAR_LABEL_INFOS, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_CODE_WARNINGS, "Warning(s)", BAR_LABEL_WARNINGS, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_CODE_ERRORS, "Error(s)", ICON_FONT_SPACE_INVADERS, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_CODE_DEBUG, "Debug(s)", BAR_LABEL_ERROR, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    ez::Log::Instance()->setStandardLogMessageFunctor([](const int& vType, const std::string& vMessage) {
        MessageData msg_datas;
        const auto& type = vType;
        Messaging::Instance()->AddMessage(vMessage, type, false, msg_datas, {});
    });
    Backend::Instance()->run(app);
    return 0;
}