#include "EntityBrowserWindow.h"
#include "../../EditorInterfaceGetters.h"
#include "../../../../Engine/Logger/Logger.h"

void EntityBrowserWindow::Draw(int phase){
    if(phase != 2) return;

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
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushID(entityId);
    ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
    tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;    
    tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;   

    if(entityId == currentEntityId)
        tree_flags |= ImGuiTreeNodeFlags_Selected;

    //if (node->Childs.Size == 0)
        tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

    auto entity = EditorInterfaceGetters::engine->GetECSManagerPtr()->GetEntity(entityId);
    auto entityName = entity->GetName();
    bool entityElement = ImGui::TreeNodeEx("", tree_flags, "%s",  entityName.c_str());

    if (ImGui::IsItemFocused())
        currentEntityId = entity->GetId();

    if (entityElement)
    {
        //for (ExampleTreeNode* child : node->Childs)
          //  DrawTreeNode(child);
        ImGui::TreePop();
    }

    ImGui::PopID();
};