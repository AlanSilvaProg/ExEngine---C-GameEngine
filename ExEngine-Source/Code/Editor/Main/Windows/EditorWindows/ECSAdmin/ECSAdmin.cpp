#include "ECSAdmin.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Core/ECS/InternalRegistry/ComponentRegistry.h"
#include "../../../../../Engine/Core/ECS/InternalRegistry/SystemRegistry.h"
#include "../../../../Utils/FileSystemOpener.h"
#include <imgui.h>
#include <string>
#include <cstring>

ECSAdmin::ECSAdmin(){
    ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    showWorldEntities = true;
    showRenameDialog = false;
    memset(renameBuffer, 0, sizeof(renameBuffer));
    systemToRename = nullptr;

    // Initialize deferred move operation variables
    pendingMoveOperation = false;
    pendingMoveFromContext = SystemContext::UPDATE;
    pendingMoveToContext = SystemContext::UPDATE;
    pendingMoveSystemTypeId = nullptr;
    pendingMoveSystem = nullptr;
};

void ECSAdmin::Draw(int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::ecsAdministratorEnabled) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(800, 400), ImVec2(FLT_MAX, FLT_MAX));
    WindowSizeManager::ApplyConstraintWithValidatedSize("Entity Component System Administrator", ImVec2(1200, 600), ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    
    if(ImGui::Begin("Entity Component System Administrator", &EditorInterfaceGetters::ecsAdministratorEnabled, windowFlags)) // 0
    {
        editECSystemTriggered = false;

        if(!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        {
            EditorInterfaceGetters::ecsAdministratorEnabled = false;
        }

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

            if(ImGui::BeginTable("SystemContextTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingStretchSame))
            {
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
                DrawColumnElement(SystemContext::EARLY_UPDATE);

                // UPDATE column
                ImGui::TableSetColumnIndex(1);
                DrawColumnElement(SystemContext::UPDATE);

                // FIXED_UPDATE column
                ImGui::TableSetColumnIndex(2);
                DrawColumnElement(SystemContext::FIXED_UPDATE);

                // LATE_UPDATE column
                ImGui::TableSetColumnIndex(3);
                DrawColumnElement(SystemContext::LATE_UPDATE);

                // PRE_RENDER column
                ImGui::TableSetColumnIndex(4);
                DrawColumnElement(SystemContext::PRE_RENDER);

                // POST_RENDER column
                ImGui::TableSetColumnIndex(5);
                DrawColumnElement(SystemContext::POST_RENDER);

                ImGui::EndTable();
            }
        }

        if(showWorldEntities)
        {
            auto entitiesText = "World Entities";
            auto entitiesTextSize = ImGui::CalcTextSize(entitiesText).x;
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x /2) - (entitiesTextSize / 2));
            ImGui::Text("%s", entitiesText);
            
            //ToDo Add existing world entities display code here if needed
        }

        if(editECSystemTriggered && editingSystem != nullptr)
        {
            ImGui::OpenPopup("Edit System");
        }

        if(showRenameDialog)
        {
            ImGui::OpenPopup("Rename System");
            showRenameDialog = false;
        }

        //Draw Edtiting system Panel if needed
        DrawEditSystemPanel();
        // Draw rename dialog if needed
        DrawRenameDialog();
    }
    ImGui::End(); // 0
    
    // Process any pending move operations after UI iteration is complete
    ProcessPendingMoveOperation();
};

void ECSAdmin::DrawColumnElement(const SystemContext currentContext)
{
    auto ecsystemContext = ecsManager->GetECSystemContext(currentContext);
    if(ecsystemContext)
    {
        for(const auto& systemEntry : ecsystemContext->GetContextSystems())
        {
            if(!includeInternals && ecsystemContext->IsInternal()) continue;
            DrawSystemWithContextMenu(&systemEntry.type, systemEntry.system, currentContext);
        }

        for(const auto& systemEntry : ecsystemContext->GetContextCustomSystems())
        {
            if(!includeInternals && ecsystemContext->IsInternal()) continue;
            DrawSystemWithContextMenu(nullptr, systemEntry, currentContext);
        }
        DrawAddSystemButton(currentContext);
    }
};

void ECSAdmin::DrawSystemWithContextMenu(const std::type_index* systemTypeId, std::shared_ptr<ECSystem> ecsystem, SystemContext currentContext){
    auto systemName = ecsystem->SystemName();
    std::string uniqueId = std::string(systemName) + "##" + std::to_string((uintptr_t)ecsystem.get());

    ImGui::Selectable((std::string(systemName)).c_str(), false);

    if(ImGui::BeginPopupContextItem(uniqueId.c_str()))
    {
        if(ImGui::MenuItem("Edit"))
        {
            editingSystem = ecsystem;
            editECSystemTriggered = true;
        }

        // Internal engine systems (and ad-hoc CustomECSystems created without a script) have no
        // backing .hpp file, so there's nothing to open in the IDE for them.
        std::string scriptPath = EditorInterfaceGetters::scriptHotReloadManager
            ? EditorInterfaceGetters::scriptHotReloadManager->GetScriptPathForSystem(ecsystem)
            : "";

        if(!scriptPath.empty() && ImGui::MenuItem("Open in IDE"))
        {
            FileSystemOpener::OpenFileInSystemEditor(scriptPath);
        }

        if(ImGui::BeginMenu("Move To"))
        {
            DrawMoveToOption(currentContext, SystemContext::EARLY_UPDATE, systemTypeId, ecsystem);
            DrawMoveToOption(currentContext, SystemContext::UPDATE, systemTypeId, ecsystem);
            DrawMoveToOption(currentContext, SystemContext::FIXED_UPDATE, systemTypeId, ecsystem);
            DrawMoveToOption(currentContext, SystemContext::LATE_UPDATE, systemTypeId, ecsystem);
            DrawMoveToOption(currentContext, SystemContext::PRE_RENDER, systemTypeId, ecsystem);
            DrawMoveToOption(currentContext, SystemContext::POST_RENDER, systemTypeId, ecsystem);
            
            ImGui::EndMenu();
        }
        
        auto castedSystem = std::dynamic_pointer_cast<CustomECSystem>(ecsystem);
        if(castedSystem)
        {
            if(ImGui::MenuItem("Rename"))
            {
                systemToRename = castedSystem;
                strncpy(renameBuffer, castedSystem->SystemName(), sizeof(renameBuffer) - 1);
                renameBuffer[sizeof(renameBuffer) - 1] = '\0';
                showRenameDialog = true;
            }

            if(ImGui::MenuItem("Delete System"))
            {
                ecsManager->GetECSystemContext(currentContext)->UnregisterCustom(castedSystem);
                ecsManager->DestroyCustomECSystem(castedSystem);
            }
        }

        ImGui::EndPopup();
    }
};

void ECSAdmin::DrawAddSystemButton(const SystemContext currentContext){
    auto buttonId = "Add System##" + std::to_string(currentContext);
    auto popupId = "AddSystemContext##" + std::to_string(currentContext);

    if(ImGui::Button(buttonId.c_str()))
    {
        ImGui::OpenPopup(popupId.c_str());
    }

    if(ImGui::BeginPopup(popupId.c_str()))
    {
        if(SystemRegistry::systemFactories.empty())
        {
            ImGui::TextDisabled("No system scripts available");
        }

        for(const auto& systemFactoryPair : SystemRegistry::systemFactories)
        {
            auto systemId = systemFactoryPair.first;
            auto nameIt = SystemRegistry::systemsNameById.find(systemId);
            std::string systemLabel = nameIt != SystemRegistry::systemsNameById.end() ? nameIt->second : ("System Id: " + std::to_string(systemId));
            std::string menuLabel = systemLabel + "###addsystem_id_" + std::to_string(systemId);

            if(ImGui::MenuItem(menuLabel.c_str()))
            {
                AddSystemFromRegistry(systemId, currentContext);
            }
        }
        ImGui::EndPopup();
    }
};

void ECSAdmin::AddSystemFromRegistry(const unsigned int systemId, const SystemContext targetContext){
    auto factoryIt = SystemRegistry::systemFactories.find(systemId);
    if(factoryIt == SystemRegistry::systemFactories.end()) return;

    auto [typeIndex, systemInstance] = factoryIt->second(ecsManager);
    if(systemInstance == nullptr) return; // already instantiated elsewhere

    // The factory registers into the context the script declared itself with (REGISTER_SYSTEM's 2nd
    // argument) - move it into whichever step's "Add System" button was actually clicked.
    auto contextIt = SystemRegistry::systemContextById.find(systemId);
    SystemContext defaultContext = contextIt != SystemRegistry::systemContextById.end() ? contextIt->second : targetContext;

    if(defaultContext != targetContext)
    {
        ecsManager->GetECSystemContext(defaultContext)->Unregister(typeIndex, systemInstance);
        ecsManager->GetECSystemContext(targetContext)->Register(typeIndex, systemInstance);
    }
};

void ECSAdmin::DrawMoveToOption(const SystemContext currentContext, const SystemContext targetContext, const std::type_index* systemTypeId, std::shared_ptr<ECSystem> ecsystem){
    if(currentContext != targetContext)
    {
        if(ImGui::MenuItem(std::to_string(targetContext).c_str()))
        {
            // Defer the move operation to avoid iterator invalidation during UI iteration
            pendingMoveOperation = true;
            pendingMoveFromContext = currentContext;
            pendingMoveToContext = targetContext;
            pendingMoveSystemTypeId = const_cast<std::type_index*>(systemTypeId);
            pendingMoveSystem = ecsystem;
        }
    }
};

void ECSAdmin::DrawEditSystemPanel(){
    if(editingSystem == nullptr) return;
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    if(ImGui::BeginPopup("Edit System", flags))
    {
        ImGui::Text("System: %s", editingSystem->SystemName());
        ImGui::Separator();
        
        auto systemEntities = *editingSystem->GetSystemEntities();

        if(systemEntities.empty())
        {
            ImGui::Text("No entities in this system");
        }
        else
        {
            ImGui::Text("Entities (%zu):", systemEntities.size());

            auto lineHeight = ImGui::GetFrameHeight();
            
            if(ImGui::BeginChild("EntitiesList", ImVec2(0, lineHeight * (systemEntities.size() - 1) + ImGui::GetFrameHeightWithSpacing()), 0, ImGuiWindowFlags_NoScrollbar))
            {
                for(auto entity : systemEntities)
                {
                    auto entityName = entity->GetName();
                    std::string uniqueId = entityName + "##" + std::to_string((uintptr_t)entity.get());

                    ImGui::Selectable(("• " + entityName + "##" + std::to_string(entity->GetId())).c_str(), false);

                    if(ImGui::BeginPopupContextItem(uniqueId.c_str()))
                    {
                        if(ImGui::MenuItem("Delete Entity")){
                            entityToDelete = entity;
                        }

                        ImGui::EndPopup();
                    }
                }
                ImGui::EndChild();
            }
        }

        if(entityToDelete != nullptr)
        {
            entityToDelete->KillImmediately();
            entityToDelete = nullptr;
        }

        ImGui::Separator();

        ImGui::Text("System Requirements:");

        ImGui::Separator();

        if(editingSystem->GetRequirements().size() == 0)
        {
            ImGui::Text("No components required");
        }
        else
        {
            for(auto systemRequirement : editingSystem->GetRequirements())
            {
                DrawRequirement(systemRequirement);
            }
        }

        if(systemRequirementToRemove >= 0)
        {
            editingSystem->RemoveRequirement(systemRequirementToRemove);
            systemRequirementToRemove = -1;
        }
        
        ImGui::Separator();

        ImGui::Text("System Optional Requirements:");

        ImGui::Separator();

        if(editingSystem->GetRequirements(true).size() == 0)
        {
            ImGui::Text("No optional components");
        }
        else
        {
            for(auto systemRequirement : editingSystem->GetRequirements(true))
            {
                DrawRequirement(systemRequirement);
            }
        }

        ImGui::Separator();

        auto castedSystem = std::dynamic_pointer_cast<CustomECSystem>(editingSystem);

        if(castedSystem != nullptr)
        {
            auto addRequirementTxt = "Add Requirement";
            float buttonWidth = ImGui::CalcTextSize(addRequirementTxt).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            float availableWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((availableWidth - buttonWidth) * 0.5f);

            if(ImGui::Button(addRequirementTxt))
            {
                ImGui::OpenPopup("AddRequirementContext");
            }
            
            if(ImGui::BeginPopup("AddRequirementContext"))
            {   
                DrawAddRequirementOptions(castedSystem, false);
                ImGui::EndPopup();
            }

            auto addOptionalRequirementTxt = "Add Optional Requirement";
            buttonWidth = ImGui::CalcTextSize(addOptionalRequirementTxt).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            availableWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((availableWidth - buttonWidth) * 0.5f);

            if(ImGui::Button(addOptionalRequirementTxt))
            {
                ImGui::OpenPopup("AddOptionalRequirementTxt");
            }
            
            if(ImGui::BeginPopup("AddOptionalRequirementTxt"))
            {   
                DrawAddRequirementOptions(castedSystem, true);
                ImGui::EndPopup();
            }
        }
        
        auto buttonCloseTxt = "Close";
        float buttonWidth = ImGui::CalcTextSize(buttonCloseTxt).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float availableWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX((availableWidth - buttonWidth) * 0.5f);
        
        if(ImGui::Button(buttonCloseTxt))
        {
            editingSystem = nullptr;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    else
        editingSystem = nullptr;
};

void ECSAdmin::DrawRenameDialog(){
    if(systemToRename == nullptr) return;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 120), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    
    if(ImGui::BeginPopup("Rename System", flags))
    {
        ImGui::Text("Enter new name:");
        ImGui::Separator();
        
        bool enterPressed = ImGui::InputText("##rename_input", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
        
        ImGui::Separator();
        
        if(ImGui::Button("Apply") || enterPressed)
        {
            systemToRename->SetSystemName(renameBuffer);
            systemToRename = nullptr;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        
        if(ImGui::Button("Cancel"))
        {
            systemToRename = nullptr;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
};

void ECSAdmin::DrawRequirement(int systemRequirement){
    auto componentName = ComponentRegistry::componentsNameById[systemRequirement];
    auto popupLabel = componentName + "###id_" + componentName + std::to_string(systemRequirement);
    std::string uniqueId = componentName + "##id_" + componentName + std::to_string((uintptr_t)editingSystem.get());

    ImGui::Selectable(popupLabel.c_str(), false);

    if(ImGui::BeginPopupContextItem(uniqueId.c_str())){
        if(ImGui::MenuItem("Remove Requirement"))
        {
            systemRequirementToRemove = systemRequirement;
        }
        ImGui::EndPopup();
    }
};

void ECSAdmin::DrawAddRequirementOptions(std::shared_ptr<CustomECSystem>& castedSystem, const bool isOptional){
    for(auto ecsystemRequirement : ComponentRegistry::ecsystemRequirement)
    {
        auto componentName = ComponentRegistry::componentsNameById[ecsystemRequirement.first];
        auto popupLabel = componentName + "###id_" + componentName;
        
        if(ImGui::MenuItem(popupLabel.c_str())){
            ecsystemRequirement.second(castedSystem, isOptional);
            ecsManager->RevalidateSystem(editingSystem);
        }
    }
};

bool ECSAdmin::DrawIfCanMoveEntityToSystem(std::shared_ptr<EntityCS> entity, std::shared_ptr<ECSystem> system){
    if(system->CheckEntitySignatureMatch(entity->GetComponentSignature()))
    {
        std::string uniqueId = std::string(system->SystemName()) + "##" + std::to_string((uintptr_t)system.get());
        if(ImGui::MenuItem(uniqueId.c_str()))
        {
            system->ValidateEntity(entity);
            return true;
        }
    }
    return false;
}

void ECSAdmin::ProcessPendingMoveOperation(){
    if(!pendingMoveOperation) return;
    
    if(pendingMoveSystemTypeId == nullptr)
    {
        // Custom system move
        auto castedSystem = std::dynamic_pointer_cast<CustomECSystem>(pendingMoveSystem);
        if(castedSystem)
        {
            ecsManager->GetECSystemContext(pendingMoveFromContext)->UnregisterCustom(castedSystem);
            ecsManager->GetECSystemContext(pendingMoveToContext)->RegisterCustom(castedSystem);
        }
    }
    else
    {
        // Regular system move
        ecsManager->GetECSystemContext(pendingMoveFromContext)->Unregister(*pendingMoveSystemTypeId, pendingMoveSystem);
        ecsManager->GetECSystemContext(pendingMoveToContext)->Register(*pendingMoveSystemTypeId, pendingMoveSystem);
    }
    
    // Reset pending operation state
    pendingMoveOperation = false;
    pendingMoveSystemTypeId = nullptr;
    pendingMoveSystem = nullptr;
}