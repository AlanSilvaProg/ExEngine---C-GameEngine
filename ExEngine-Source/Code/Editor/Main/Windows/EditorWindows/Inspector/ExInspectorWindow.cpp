#include "ExInspectorWindow.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include "../ElementSelectionController.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Serializer/ISerializable.h"
#include "../../../../../Engine/Core/Rendering/Layer/LayerAttributes.h"

ExInspectorWindow::ExInspectorWindow(){
    ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
};

void ExInspectorWindow::Draw(int phase){
    if(phase != 1) return;

    if(ImGui::Begin("ExInspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar))
    {
        auto selectedElement = ElementSelectionController::GetCurrentSelection();

        if(selectedElement != nullptr)
        {
            if(selectedElement->GetType() == EditorSelectableType::Entity)
            {
                DrawEntity(dynamic_cast<EntityBrowserSelection*>(selectedElement));
            }
            else if(selectedElement->GetType() == EditorSelectableType::Asset)
            {
                //ToDo
                //DrawAsset();
            }
        }
    }

    ImGui::End();
};

void ExInspectorWindow::DrawEntity(EntityBrowserSelection* entityBrowserSelection){
    if(entityBrowserSelection == nullptr) return;

    const auto entityId = entityBrowserSelection->GetSelectedEntityId();
    const auto componentsPool = ecsManager->GetEntityComponentPools();
    const auto entity = ecsManager->GetEntity(entityId);

    ImGui::Text("%s", "Entity");
    ImGui::Text("%s", "Name: ");
    ImGui::SameLine();

    //generating uniqueId per field
    std::string uniqueId = "entityName###" 
    + std::to_string(entity->GetId()) + "_" 
    + std::to_string(reinterpret_cast<uintptr_t>(std::to_string(entity->GetId()).c_str()));
    ImGui::PushID(uniqueId.c_str());

    static const char* entityNameLabel = "";
    char buf[256]{};
    std::snprintf(buf, sizeof(buf), "%s", entity->GetName().c_str());
    if (ImGui::InputText(entityNameLabel, buf, sizeof(buf))) {
        entity->ChangeName(buf);
    }

    ImGui::PopID();

    ImGui::SameLine();
    ImGui::Text("%s", "Entity Id: ");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(entity->GetId()).c_str());
    ImGui::SeparatorText("Components");

    for(const auto pool : componentsPool)
    {
        DrawEntityComponent(pool, entityId);
    }
};

void ExInspectorWindow::DrawEntityComponent(const std::shared_ptr<IPool> componentPool, const int entityId){
    //casting to component
    
    auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(componentPool);

    if(castedPoolManager == nullptr) return;
    //getting entity component 
    const auto component = castedPoolManager->GetComponent(entityId);

    if(component == nullptr) return;

    if(!ecsManager->HasComponent(entityId, component->GetComponentId())) return;

    //getting serialized fields
    const auto fieldsToSerialize = component->Serialize();

    ImGui::BeginGroup();

    DrawSerializedClass(fieldsToSerialize, component->GetComponentId(), true);  

    ImGui::PushID(std::to_string(reinterpret_cast<uintptr_t>(componentPool.get())).c_str());  
    if(ImGui::SmallButton("Remove Component"))
    {
        ecsManager->GetEntity(entityId)->RemoveComponent(component->GetComponentId());
    }
    ImGui::PopID();

    ImGui::Separator();
    ImGui::EndGroup();
};

void ExInspectorWindow::DrawSerializedClass(const ExSerializedClass& fieldsToSerialize, const int id = 0, bool root = false) const
{
    //serializing fields
    auto className = fieldsToSerialize.className;
    ImGui::Text("%s", className.c_str());

    if(root)
    {
        ImGui::SameLine();
        ImGui::Text("%s", "Component Id: ");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(id).c_str());
    }

    for(auto field : fieldsToSerialize.serializedFields)
    {
        DrawComponentField(field, className);
    }
};

void ExInspectorWindow::DrawComponentField(const ExSerializedField& exSerializedField, const std::string& className) const{
    if (exSerializedField.serializable_ptr != nullptr)
    {
        DrawSerializedClass(exSerializedField.serializable_ptr->Serialize());
        return;
    }

    ImGui::Text("%s", Demangle(exSerializedField.fieldName.c_str()).c_str());
    ImGui::SameLine(0, 20);

    //generating uniqueId per field
    std::string uniqueId = exSerializedField.fieldName + "###" 
    + className + "_" 
    + std::to_string(reinterpret_cast<uintptr_t>(exSerializedField.field_ptr));
    ImGui::PushID(uniqueId.c_str());

    static const char* label = "";

    if (exSerializedField.fieldType == typeid(int)) {
        ImGui::InputInt(label, static_cast<int*>(exSerializedField.field_ptr));
    } 
    else if (exSerializedField.fieldType == typeid(float)) {
        ImGui::InputFloat(label, static_cast<float*>(exSerializedField.field_ptr));
    } 
    else if (exSerializedField.fieldType == typeid(bool)) {
        ImGui::Checkbox(label, static_cast<bool*>(exSerializedField.field_ptr));
    } 
    else if (exSerializedField.fieldType == typeid(std::string)) {
        auto* s = static_cast<std::string*>(exSerializedField.field_ptr);
        char buf[256]{};
        std::snprintf(buf, sizeof(buf), "%s", s->c_str());
        if (ImGui::InputText(label, buf, sizeof(buf))) {
            *s = buf;
        }
    }
    else if (exSerializedField.fieldType == typeid(glm::vec3)) {
        glm::vec3* v = static_cast<glm::vec3*>(exSerializedField.field_ptr);
        ImGui::InputFloat3(label, &(*v)[0]);
    }
    else
    {
        ImGui::Text("Unsupported type: %s", exSerializedField.fieldName.c_str());
    }

    ImGui::PopID();
};

//void ExInspectorWindow::DrawAsset(){
//
//};