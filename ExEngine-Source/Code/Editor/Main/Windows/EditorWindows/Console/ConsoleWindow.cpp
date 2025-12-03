#include "ConsoleWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Logger/LoggerNotifier.h"
#include <imgui.h>

ConsoleWindow::ConsoleWindow(){
    *LoggerNotifier::newLogEntryHandler += [this](std::shared_ptr<LogEntry> entry) { logEntries.push_back(entry); };

    logTypeColor.emplace(LogType::LOG, Color::WHITE);
    logTypeColor.emplace(LogType::WARNING, Color::GREEN);
    logTypeColor.emplace(LogType::ERROR, Color::RED);
    
    for(auto entry : Logger::GetAllLogEntries())
    {
        logEntries.push_back(entry);
    }
};

void ConsoleWindow::Draw(int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::consoleEnabled) return;

    ImGui::SetNextWindowSizeConstraints({100, consoleContentSizeY}, ImGui::GetWindowSize());
    if(ImGui::Begin("Console", &EditorInterfaceGetters::consoleEnabled, ImGuiWindowFlags_MenuBar)) //0
    {
    
        if(ImGui::BeginMenuBar()) // 1
        {
            if(ImGui::MenuItem("Clear"))
            {
                logEntries.clear();
                selectedLogIndex = -1;
            }

            ImGui::MenuItem("Clear on play", nullptr, &clearOnPlay);
            ImGui::MenuItem("Show Log", nullptr, &showLog);
            ImGui::MenuItem("Show Warning", nullptr, &showWarning);
            ImGui::MenuItem("Show Error", nullptr, &showError);

            ImGui::EndMenuBar(); // 1
        }

        auto availableSpace = ImGui::GetContentRegionAvail();
        auto consoleBgColor = Color::GREY;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, {consoleBgColor->r / 255.0f, consoleBgColor->g / 255.0f, consoleBgColor->b / 255.0f, consoleBgColor->a/ 255.0f}); 
        if(ImGui::BeginChild("Console Content", {availableSpace.x, availableSpace.y * 0.8f}, ImGuiChildFlags_Borders)) // 2
        {
            int index = 0;
            for (auto entry : logEntries)
            {
                switch (entry->logType)
                {
                case LogType::LOG:
                    if(!showLog) continue;
                    break;
                case LogType::WARNING:
                    if(!showWarning) continue;
                    break;
                case LogType::ERROR:
                    if(!showError) continue;
                    break;
                }

                if (index == 0)
                    ImGui::Separator();

                auto color = logTypeColor[entry->logType];

                // pega apenas as duas primeiras linhas
                std::string preview = GetPreview(entry->message);

                // altura fixa
                float itemHeight = ImGui::GetTextLineHeight() * 2.2f;

                ImGui::PushID(index);

                bool isSelected = (selectedLogIndex == index);
                if (ImGui::Selectable("##logitem", isSelected, 0, ImVec2(0, itemHeight)))
                {
                    selectedLogIndex = index;
                }

                // desloca cursor para dentro do item
                ImVec2 cursor = ImGui::GetItemRectMin();
                cursor.x += 4;
                cursor.y += 4;
                ImGui::SetCursorScreenPos(cursor);

                // cor do texto
                ImGui::PushStyleColor(ImGuiCol_Text,
                    ImVec4(color->r, color->g, color->b, color->a));

                // exibe sem wrap
                ImGui::TextUnformatted(preview.c_str());
                ImGui::Dummy(ImVec2(0, itemHeight - ImGui::GetTextLineHeight()));

                ImGui::PopStyleColor();

                ImGui::PopID();
                ImGui::Separator();

                index++;
            }
        }
        ImGui::PopStyleColor();
        ImGui::EndChild(); // 2

        ImGui::PushStyleColor(ImGuiCol_ChildBg, {consoleBgColor->r / 255.0f, consoleBgColor->g / 255.0f, consoleBgColor->b / 255.0f, consoleBgColor->a/ 255.0f}); 
        if(ImGui::BeginChild("Console details", {availableSpace.x, availableSpace.y * 0.2f}, ImGuiChildFlags_Borders))// 3
        {
            if(selectedLogIndex >= 0 && logEntries.size() > selectedLogIndex)
            {
                auto entry = logEntries[selectedLogIndex];

                if(entry != nullptr)
                {
                    if(entry->logType == LogType::LOG && !showLog || 
                        entry->logType == LogType::WARNING && !showWarning || 
                        entry->logType == LogType::ERROR && !showError) 
                        selectedLogIndex = -1;
                    else
                    {
                        std::string content = entry->message + "\n\n" + entry->stackTrace;

                        ImGui::PushTextWrapPos();
                        ImGui::TextUnformatted(content.c_str());
                        ImGui::PopTextWrapPos();

                        if(ImGui::IsWindowHovered() && ImGui::IsKeyDown(ImGuiKey_MouseRight))
                        {
                            ImGui::OpenPopup("##context");
                        }

                        if (ImGui::BeginPopup("##context"))
                        {
                            if (ImGui::MenuItem("Copiar"))
                            {
                                if (selectedLogIndex >= 0 && selectedLogIndex < logEntries.size())
                                {
                                    ImGui::SetClipboardText(content.c_str());
                                }
                            }
                            ImGui::EndPopup();
                        }
                    }
                }
            }
        }
        ImGui::PopStyleColor();
        ImGui::EndChild(); // 3

        if (selectedLogIndex >= 0 &&
            ImGui::IsWindowFocused() &&
            (ImGui::Shortcut(ImGuiMod_Ctrl, ImGuiKey_C) || ImGui::Shortcut(ImGuiMod_Super, ImGuiKey_C)))
        {
            ImGui::SetClipboardText(logEntries[selectedLogIndex]->message.c_str());
        }
    }

    ImGui::End(); // 0
};

std::string ConsoleWindow::GetPreview(const std::string& msg)
{
    size_t first = msg.find('\n');
    if (first == std::string::npos) return msg; // só 1 linha

    size_t second = msg.find('\n', first + 1);
    if (second == std::string::npos) 
        return msg.substr(0, first); // só 2 linhas

    return msg.substr(0, second); // exatamente 2 linhas
}
