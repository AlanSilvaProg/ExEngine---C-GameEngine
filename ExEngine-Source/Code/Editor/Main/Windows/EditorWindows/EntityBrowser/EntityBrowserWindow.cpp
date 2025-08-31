#include "EntityBrowserWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"

EntityBrowserWindow::EntityBrowserWindow(){
    entityBrowserSelection = std::make_unique<EntityBrowserSelection>();
};

void EntityBrowserWindow::Draw(int phase){
    if(phase != 1) return;

    //ToDo include the currently scene name
    if(!ImGui::Begin("Scene Inspection", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::EndChild();
        Logger::LogError("unexpected error occurred when opened Scene Inspection Window");
        return;
    }

    auto aliveEntities = EditorInterfaceGetters::engine->GetECSManagerPtr()->GetAliveEntities();

    if(!ImGui::BeginChild("Scene Entities", ImVec2(300, 0), ImGuiChildFlags_ResizeX))
    {
        ImGui::End();
        Logger::LogError("unexpected error occurred when opened Scene Entity Inspection");
        return;
    }


    if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
    {
        for(auto entityId : aliveEntities)
        {
            DrawEntity(entityId);
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

    auto entityName = entity->GetName();
    bool entityElement = ImGui::TreeNodeEx("", tree_flags, "%s",  entityName.c_str());

    if (ImGui::IsItemFocused())
        entityBrowserSelection->SetEntitySelected(entity->GetId());

    if(entityId == entityBrowserSelection->GetSelectedEntityId())
        tree_flags |= ImGuiTreeNodeFlags_Selected;

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