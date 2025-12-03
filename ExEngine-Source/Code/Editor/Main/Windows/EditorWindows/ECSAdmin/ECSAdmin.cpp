#include "ECSAdmin.h"
#include "../../../EditorInterfaceGetters.h"
#include <imgui.h>
#include <string>

ECSAdmin::ECSAdmin(){
    ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    showWorldEntities = true;
};

void ECSAdmin::Draw(int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::ecsAdministratorEnabled) return;

    if(ImGui::Begin("Entity Component System Administrator", &EditorInterfaceGetters::ecsAdministratorEnabled, ImGuiWindowFlags_MenuBar)) // 0
    {
        if(ImGui::BeginMenuBar()) // 1
        {
            ImGui::Separator();
            if(ImGui::MenuItem("World Entities", nullptr, &showWorldEntities))
            {
                showSystems = false;
            }
            
            ImGui::Separator();
            if(ImGui::MenuItem("ECS - Systems", nullptr, &showSystems))
            {
                showWorldEntities = false;
            }

            ImGui::Separator();
            if(ImGui::MenuItem("Show Internals", nullptr, &includeInternals))
            {

            }
            ImGui::Separator();

            ImGui::EndMenuBar(); // 1
        }

        const auto ecsSystems = ecsManager->GetAllSystems();
        auto systemsText = "ECS - Systems";
        auto systemsTextSize = ImGui::CalcTextSize(systemsText).x;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x /2) - (systemsTextSize / 2));
        ImGui::Text("%s", systemsText);

        //ToDo editor to create systems and vinculate components within it
        for(const auto& ecsSystem : ecsSystems)
        {
            ImGui::Text("%s", ecsSystem.second->SystemName());
        }
    }
    ImGui::End(); // 0
};