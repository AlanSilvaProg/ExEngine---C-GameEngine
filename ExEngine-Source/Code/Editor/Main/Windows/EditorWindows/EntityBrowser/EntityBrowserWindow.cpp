#include "EntityBrowserWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../EditorEvents/EditorCommandEventHandler.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Scene/ECSWorldManager.h"
#include <algorithm>
#include <vector>

EntityBrowserWindow::EntityBrowserWindow(){
    entityBrowserSelection = std::make_unique<EntityBrowserSelection>();
    *EditorCommandEventHandler::duplicate += [this](){ this->Duplicate(); };
    *EditorCommandEventHandler::deleteCmmd += [this](){ this->Delete(); };
    EditorInterfaceGetters::entityBrowserEnabled = true; // ToDo -> control with persistence
};

void EntityBrowserWindow::Duplicate(){
    if (EntityBrowserWindow::IsValidSelection())
    {
        EditorInterfaceGetters::engine->GetECSManagerPtr()->DuplicateEntity(entityBrowserSelection->GetSelectedEntityId());
    }
};

void EntityBrowserWindow::Delete(){
    if (EntityBrowserWindow::IsValidSelection())
    {
        EditorInterfaceGetters::engine->GetECSManagerPtr()->DestroyEntity(entityBrowserSelection->GetSelectedEntityId());
    }
};

bool EntityBrowserWindow::IsValidSelection(){
    if(selectionDetected)
    {
        auto currentElementSelected = ElementSelectionController::GetCurrentSelection();
        if(currentElementSelected != nullptr)
        {
            if(currentElementSelected == entityBrowserSelection.get())
            {
                return true;
            }
        }
    }
    return false;
};

void EntityBrowserWindow::Draw(int phase){
    if(phase != 1) return;

    bool scriptsStillCompiling = EditorInterfaceGetters::scriptHotReloadManager != nullptr
        && EditorInterfaceGetters::scriptHotReloadManager->IsCompiling();

    if(!ECSWorldManager::HasCurrentWorld() && !scriptsStillCompiling)
    {
        if(EditorInterfaceGetters::currentWorldPath != "")
        {
            ECSWorldManager::LoadWorld(EditorInterfaceGetters::currentWorldPath);
        }
        else
        {
            ECSWorldManager::GenerateWorld();
            EditorInterfaceGetters::worldWithoutPath = true;
        }
    }

    if(!EditorInterfaceGetters::entityBrowserEnabled) return;
    if(!ECSWorldManager::HasCurrentWorld()) return; // world not ready yet (e.g. scripts still compiling)

    auto inspectionLabel = "World Inspection - " + ECSWorldManager::GetCurrentWorldInfo().name;

    // Apply minimum size constraint using WindowSizeManager
    WindowSizeManager::ApplyConstraintWithValidatedSize(inspectionLabel.c_str(), ImVec2(400, 500), ImGuiCond_FirstUseEver);

    if(!ImGui::Begin(inspectionLabel.c_str(), &EditorInterfaceGetters::entityBrowserEnabled, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    auto& aliveEntities = EditorInterfaceGetters::engine->GetECSManagerPtr()->GetAliveEntities();

    std::vector<int> visibleEntities;
    visibleEntities.reserve(aliveEntities.size());
    for(auto entityId : aliveEntities)
    {
        visibleEntities.push_back(entityId);
    }
    std::sort(visibleEntities.begin(), visibleEntities.end());

    if(!ImGui::BeginChild("World Entities", ImVec2(300, 0), ImGuiChildFlags_ResizeX))
    {
        ImGui::EndChild();
        ImGui::End();
        return;
    }

    NavigateSelectionWithArrows(visibleEntities);

    if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
    {
        for(auto entityId : visibleEntities)
        {
            DrawEntity(entityId);
        }

        if (selectionDetected)
        {
            if (ImGui::IsWindowHovered() && ImGui::IsAnyMouseDown() && !ImGui::IsAnyItemHovered())
            {
                selectionDetected = false;
                ElementSelectionController::SetSelected(nullptr);//null selection
            }
        }
        if(!selectionDetected)
        {
            CheckContextWindowWithoutSelection();
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();

    ImGui::End();
};

void EntityBrowserWindow::NavigateSelectionWithArrows(const std::vector<int>& visibleEntities){
    if(visibleEntities.empty()) return;
    if(!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) return;

    bool down = ImGui::IsKeyPressed(ImGuiKey_DownArrow);
    bool up = ImGui::IsKeyPressed(ImGuiKey_UpArrow);
    if(!down && !up) return;

    int newIndex = 0;
    if(IsValidSelection())
    {
        auto it = std::find(visibleEntities.begin(), visibleEntities.end(), entityBrowserSelection->GetSelectedEntityId());
        int currentIndex = (it != visibleEntities.end()) ? (int)std::distance(visibleEntities.begin(), it) : 0;
        newIndex = std::clamp(currentIndex + (down ? 1 : -1), 0, (int)visibleEntities.size() - 1);
    }

    entityBrowserSelection->SetEntitySelected(visibleEntities[newIndex]);
    selectionDetected = true;
};

void EntityBrowserWindow::DrawEntity(int entityId){
    auto entity = EditorInterfaceGetters::engine->GetECSManagerPtr()->GetEntity(entityId);

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushID(entityId);
    ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
    tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;    
    tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;   

    bool rightClick = ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
    (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::GetIO().KeyCtrl);

    if (entityId == entityBrowserSelection->GetSelectedEntityId() && ElementSelectionController::GetCurrentSelection() != nullptr)
        tree_flags |= ImGuiTreeNodeFlags_Selected;

    auto entityName = entity->GetName();
    bool entityElement = ImGui::TreeNodeEx("", tree_flags, "%s",  entityName.c_str());

    if (ImGui::IsItemClicked() ||
    ImGui::IsItemHovered() && rightClick)
    {
        entityBrowserSelection->SetEntitySelected(entity->GetId());
        selectionDetected = true;
    }

    if(entityId == entityBrowserSelection->GetSelectedEntityId())
    {
        if(ImGui::BeginPopupContextItem())
        {
            if(ImGui::MenuItem("Delete"))
            {
                ECSWorldManager::GetCurrentWorld()->DetachEntity(entity);
                entity->Kill();
                selectionDetected = false;
                ElementSelectionController::SetSelected(nullptr);//null selection
            }
            ImGui::EndPopup();
        }
    }

    //if (node->Childs.Size == 0)
        tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

    if (entityElement)
    {
        //for (ExampleTreeNode* child : node->Childs)
          //  DrawTreeNode(child);
        ImGui::TreePop();
    }

    ImGui::PopID();
};

void EntityBrowserWindow::CheckContextWindowWithoutSelection(){
    if(ImGui::BeginPopupContextWindow())
    {
        if(ImGui::MenuItem("Create new Entity"))
        {
            auto entity = EditorInterfaceGetters::engine->GetECSManagerPtr()->CreateEntity(defaultEntityName);
            ECSWorldManager::GetCurrentWorld()->AttachEntity(entity);
        }
        ImGui::EndPopup();
    }
};