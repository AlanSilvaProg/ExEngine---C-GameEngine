#pragma once
#include "../../EditorWindow.h"
#include "../EngineConfig/WindowSizeManager.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Utils/Color.h"
#include <vector>
#include <map>
#include <memory>
#include <string>

class ConsoleWindow: public EditorWindow{
private:
    bool clearOnPlay = true;
    bool showLog = true;
    bool showWarning;
    bool showError;

    int selectedLogIndex = -1;
    float consoleContentSizeY = 200;
    float bottomPaneHeight = 150.0f;

    LogEntry selectedEntry;

    std::vector<std::shared_ptr<LogEntry>> logEntries;
    std::map<LogType, std::shared_ptr<ColorChannel>> logTypeColor;

    std::string GetPreview(const std::string& msg);
public:
    ConsoleWindow();
    void Draw(const int phase) override; //0 == early 1 == late
};