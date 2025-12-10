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

        if(showSystems)
        {
            const auto ecsSystems = ecsManager->GetAllSystems();
            auto systemsText = "ECS - Systems Context";
            auto systemsTextSize = ImGui::CalcTextSize(systemsText).x;
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x /2) - (systemsTextSize / 2));
            ImGui::Text("%s", systemsText);

            ImGui::Separator();

            // Create columns for each SystemContext
            if(ImGui::BeginTable("SystemContextTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX))
            {
                // Setup column headers
                ImGui::TableSetupColumn("EARLY_UPDATE");
                ImGui::TableSetupColumn("UPDATE");
                ImGui::TableSetupColumn("FIXED_UPDATE");
                ImGui::TableSetupColumn("LATE_UPDATE");
                ImGui::TableSetupColumn("PRE_RENDER");
                ImGui::TableSetupColumn("POST_RENDER");
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();

                // EARLY_UPDATE column
                ImGui::TableSetColumnIndex(0);
                auto earlyUpdateContext = ecsManager->GetECSystemContext(SystemContext::EARLY_UPDATE);
                if(earlyUpdateContext)
                {
                    const auto& systems = earlyUpdateContext->GetContextSystems();
                    for(const auto& systemEntry : systems)
                    {
                        if(!includeInternals && earlyUpdateContext->IsInternal()) continue;
                        DrawSystemWithContextMenu(systemEntry, SystemContext::EARLY_UPDATE);
                    }
                }

                // UPDATE column
                ImGui::TableSetColumnIndex(1);
                auto updateContext = ecsManager->GetECSystemContext(SystemContext::UPDATE);
                if(updateContext)
                {
                    const auto& systems = updateContext->GetContextSystems();
                    for(const auto& systemEntry : systems)
                    {
                        if(!includeInternals && updateContext->IsInternal()) continue;
                        DrawSystemWithContextMenu(systemEntry, SystemContext::UPDATE);
                    }
                }

                // FIXED_UPDATE column
                ImGui::TableSetColumnIndex(2);
                auto fixedUpdateContext = ecsManager->GetECSystemContext(SystemContext::FIXED_UPDATE);
                if(fixedUpdateContext)
                {
                    const auto& systems = fixedUpdateContext->GetContextSystems();
                    for(const auto& systemEntry : systems)
                    {
                        if(!includeInternals && fixedUpdateContext->IsInternal()) continue;
                        DrawSystemWithContextMenu(systemEntry, SystemContext::FIXED_UPDATE);
                    }
                }

                // LATE_UPDATE column
                ImGui::TableSetColumnIndex(3);
                auto lateUpdateContext = ecsManager->GetECSystemContext(SystemContext::LATE_UPDATE);
                if(lateUpdateContext)
                {
                    const auto& systems = lateUpdateContext->GetContextSystems();
                    for(const auto& systemEntry : systems)
                    {
                        if(!includeInternals && lateUpdateContext->IsInternal()) continue;
                        DrawSystemWithContextMenu(systemEntry, SystemContext::LATE_UPDATE);
                    }
                }

                // PRE_RENDER column
                ImGui::TableSetColumnIndex(4);
                auto preRenderContext = ecsManager->GetECSystemContext(SystemContext::PRE_RENDER);
                if(preRenderContext)
                {
                    const auto& systems = preRenderContext->GetContextSystems();
                    for(const auto& systemEntry : systems)
                    {
                        if(!includeInternals && preRenderContext->IsInternal()) continue;
                        DrawSystemWithContextMenu(systemEntry, SystemContext::PRE_RENDER);
                    }
                }

                // POST_RENDER column
                ImGui::TableSetColumnIndex(5);
                auto postRenderContext = ecsManager->GetECSystemContext(SystemContext::POST_RENDER);
                if(postRenderContext)
                {
                    const auto& systems = postRenderContext->GetContextSystems();
                    for(const auto& systemEntry : systems)
                    {
                        if(!includeInternals && postRenderContext->IsInternal()) continue;
                        DrawSystemWithContextMenu(systemEntry, SystemContext::POST_RENDER);
                    }
                }

                ImGui::EndTable();
            }
        }

        if(showWorldEntities)
        {
            auto entitiesText = "World Entities";
            auto entitiesTextSize = ImGui::CalcTextSize(entitiesText).x;
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x /2) - (entitiesTextSize / 2));
            ImGui::Text("%s", entitiesText);
            
            // Add your existing world entities display code here if needed
        }

        
    }
    ImGui::End(); // 0
};

void ECSAdmin::DrawSystemWithContextMenu(const SystemEntry& systemEntry, SystemContext currentContext)
{
    const char* systemName = systemEntry.system->SystemName();
    std::string uniqueId = std::string(systemName) + "##" + std::to_string((uintptr_t)systemEntry.system.get());
    
    ImGui::Selectable(("• " + std::string(systemName)).c_str(), false);
    
    if(ImGui::BeginPopupContextItem(uniqueId.c_str()))
    {
        if(ImGui::BeginMenu("Move To"))
        {
            // EARLY_UPDATE option
            if(currentContext != SystemContext::EARLY_UPDATE)
            {
                if(ImGui::MenuItem("EARLY_UPDATE"))
                {
                    // TODO: Implement move system to EARLY_UPDATE context
                }
            }
            
            // UPDATE option
            if(currentContext != SystemContext::UPDATE)
            {
                if(ImGui::MenuItem("UPDATE"))
                {
                    // TODO: Implement move system to UPDATE context
                }
            }
            
            // FIXED_UPDATE option
            if(currentContext != SystemContext::FIXED_UPDATE)
            {
                if(ImGui::MenuItem("FIXED_UPDATE"))
                {
                    // TODO: Implement move system to FIXED_UPDATE context
                }
            }
            
            // LATE_UPDATE option
            if(currentContext != SystemContext::LATE_UPDATE)
            {
                if(ImGui::MenuItem("LATE_UPDATE"))
                {
                    // TODO: Implement move system to LATE_UPDATE context
                }
            }
            
            // PRE_RENDER option
            if(currentContext != SystemContext::PRE_RENDER)
            {
                if(ImGui::MenuItem("PRE_RENDER"))
                {
                    // TODO: Implement move system to PRE_RENDER context
                }
            }
            
            // POST_RENDER option
            if(currentContext != SystemContext::POST_RENDER)
            {
                if(ImGui::MenuItem("POST_RENDER"))
                {
                    // TODO: Implement move system to POST_RENDER context
                }
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::EndPopup();
    }
}