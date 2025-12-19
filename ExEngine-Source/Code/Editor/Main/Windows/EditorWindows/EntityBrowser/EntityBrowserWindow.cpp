#include "EntityBrowserWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Scene/ECSWorldManager.h"

EntityBrowserWindow::EntityBrowserWindow(){
    entityBrowserSelection = std::make_unique<EntityBrowserSelection>();
};

void EntityBrowserWindow::Draw(int phase){
    if(phase != 1) return;

    if(!ECSWorldManager::HasCurrentWorld())
    {
        ECSWorldManager::GenerateWorld();
        EditorInterfaceGetters::worldWithoutPath = true;
    }

    // Apply minimum size constraint using WindowSizeManager
    WindowSizeManager::ApplyMinimumSizeConstraint("World Inspection");
    
    //ToDo include the currently scene name
    if(!ImGui::Begin("World Inspection", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    auto aliveEntities = EditorInterfaceGetters::engine->GetECSManagerPtr()->GetAliveEntities();

    if(!ImGui::BeginChild("World Entities", ImVec2(300, 0), ImGuiChildFlags_ResizeX))
    {
        ImGui::EndChild();
        ImGui::End();
        return;
    }


    if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
    {
        for(auto entityId : aliveEntities)
        {
            DrawEntity(entityId);
        }
         
        if (selectionDetected && ImGui::IsWindowHovered() && ImGui::IsAnyMouseDown())
        {
            if (!ImGui::IsAnyItemHovered())
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

void EntityBrowserWindow::DrawEntity(int entityId){
    auto entity = EditorInterfaceGetters::engine->GetECSManagerPtr()->GetEntity(entityId);

    if(entity->IsInternal()) return;

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