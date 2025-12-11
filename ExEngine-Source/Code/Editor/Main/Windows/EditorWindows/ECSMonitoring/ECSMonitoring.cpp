#include "ECSMonitoring.h"
#include "../../../EditorInterfaceGetters.h"
#include "../EngineConfig/WindowSizeManager.h"
#include <imgui.h>

ECSMonitoring::ECSMonitoring(){
    enginePtr = EditorInterfaceGetters::engine;
    ecsManagerPtr = enginePtr->GetECSManagerPtr();
};

void ECSMonitoring::Draw(int phase){
    if(phase != 1 || !EditorInterfaceGetters::ecsMonitoringEnabled) return;

    // Apply minimum size constraint and validate initial size using WindowSizeManager
    ImVec2 standardSize = {600, 600};
    auto availableSpace = ImGui::GetContentRegionAvail();

    if(standardSize.x > availableSpace.x)
        standardSize.x = availableSpace.x;
    if(standardSize.y > availableSpace.y)
        standardSize.y = availableSpace.y;

    WindowSizeManager::ApplyConstraintWithValidatedSize("Ecs Monitoring Panel", standardSize, ImGuiCond_Appearing);
    if(ImGui::Begin("Ecs Monitoring Panel", &EditorInterfaceGetters::ecsMonitoringEnabled)) //0
    {
        auto aliveEntities = ecsManagerPtr->GetAliveEntities();
        ImGui::Text("%s", std::string("Alive entities: " + std::to_string(aliveEntities.size())).c_str());
        //ToDo Add monitoring stats for existent entities.. memory profilling.. etc..
    }

    ImGui::End(); // 0
};