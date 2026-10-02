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
    if (ElementSelectionController::IsDeleteCommandOverridden()) return;

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

void EntityBrowserWindow::Draw(const int phase){
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
        auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
        for(auto entityId : visibleEntities)
        {
            // Children are drawn recursively by their own parent node below, not flattened here.
            auto entity = ecsManager->GetEntity(entityId);
            if(entity != nullptr && entity->GetParentId() != static_cast<unsigned int>(-1)) continue;

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

    // Drop zone for the leftover empty space below the tree - dragging an entity here detaches it
    // from its current parent instead of re-parenting it onto something.
    ImGui::InvisibleButton("##EntityHierarchyEmptySpace", ImGui::GetContentRegionAvail());
    if(ImGui::BeginDragDropTarget()){
        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityHierarchyDrag")){
            const int draggedEntityId = *static_cast<const int*>(payload->Data);
            auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
            auto draggedEntity = ecsManager->GetEntity(draggedEntityId);

            if(draggedEntity != nullptr){
                const auto currentParentId = draggedEntity->GetParentId();
                if(currentParentId != static_cast<unsigned int>(-1))
                    ecsManager->RemoveChildren(currentParentId, draggedEntityId);
            }
        }
        ImGui::EndDragDropTarget();
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
    auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    auto entity = ecsManager->GetEntity(entityId);
    if(entity == nullptr) return;

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushID(entityId);

    const auto& children = entity->GetChildrens();

    ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
    tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
    if(children.empty())
        tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

    bool rightClick = ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
    (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::GetIO().KeyCtrl);

    if (entityId == entityBrowserSelection->GetSelectedEntityId() && ElementSelectionController::GetCurrentSelection() != nullptr)
        tree_flags |= ImGuiTreeNodeFlags_Selected;

    auto entityName = entity->GetName();

    // Inactive (or inside an inactive ancestor, which cascades) reads as dimmed, matching how it
    // behaves at runtime - stopped, not just visually hidden.
    const bool isEntityEnabled = entity->IsEnabled();
    if(!isEntityEnabled) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    bool entityElement = ImGui::TreeNodeEx("", tree_flags, "%s",  entityName.c_str());
    if(!isEntityEnabled) ImGui::PopStyleColor();

    if (ImGui::IsItemClicked() ||
    ImGui::IsItemHovered() && rightClick)
    {
        entityBrowserSelection->SetEntitySelected(entity->GetId());
        selectionDetected = true;
    }

    // Drag this node onto another one to re-parent it there.
    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
        ImGui::SetDragDropPayload("EntityHierarchyDrag", &entityId, sizeof(int));
        ImGui::Text("%s", entityName.c_str());
        ImGui::EndDragDropSource();
    }

    if(ImGui::BeginDragDropTarget()){
        if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityHierarchyDrag")){
            const int draggedEntityId = *static_cast<const int*>(payload->Data);
            if(draggedEntityId != entityId)
                ecsManager->SetParent(entityId, draggedEntityId);
        }
        ImGui::EndDragDropTarget();
    }

    if(entityId == entityBrowserSelection->GetSelectedEntityId())
    {
        if(ImGui::BeginPopupContextItem())
        {
            if(ImGui::MenuItem("Create Child Entity"))
            {
                auto child = ecsManager->CreateEntity(defaultEntityName);
                ecsManager->SetParent(entityId, child->GetId());
                ECSWorldManager::GetCurrentWorld()->AttachEntity(child);
            }
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

    if (entityElement)
    {
        for(auto childId : children)
            DrawEntity(static_cast<int>(childId));

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