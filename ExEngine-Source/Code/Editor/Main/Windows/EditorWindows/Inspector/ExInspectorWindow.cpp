#include "ExInspectorWindow.h"
#include "../ElementSelectionController.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Serializer/ISerializable.h"
#include "../../../../../Engine/Core/Rendering/Layer/LayerAttributes.h"
#include "../../../../../Engine/Core/ECS/Component/EComponentS.h"
#include "../../../../../Engine/Core/Components/TransformComponent.h"
#include "../../../../../Engine/Core/ECS/InternalRegistry/ComponentRegistry.h"
#include "../../../../../Engine/Core/Utils/Algorithms/ExMath.h"
#include "../../../../../Engine/Core/Scene/ECSWorldManager.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <SDL.h>

ExInspectorWindow::ExInspectorWindow(){
    ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
};

void ExInspectorWindow::Draw(int phase){
    if(phase != 1) return;

    spriteInformations.clear();

    // Apply minimum size constraint using WindowSizeManager
    WindowSizeManager::ApplyMinimumSizeConstraint("ExInspector");
    
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
                DrawAsset(dynamic_cast<AssetBrowserSelection*>(selectedElement));
            }
        }
    }

    ImGui::End();
};

void ExInspectorWindow::DrawEntity(const EntityBrowserSelection* entityBrowserSelection){
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

    DrawAddComponentButton(entityId);
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

void ExInspectorWindow::DrawAddComponentButton(const int entityId){
    if(ImGui::Button("Add Component"))
    {
        ImGui::OpenPopup("AddComponentContext");
    }

    if(ImGui::BeginPopup("AddComponentContext"))
    {   
        for(auto componentRegistryPair : ComponentRegistry::components)
        {
            auto componentName = ComponentRegistry::componentsNameById[componentRegistryPair.first];
            auto popupLabel = componentName + "###id_" + componentName;
            if(ImGui::MenuItem(popupLabel.c_str())){
                componentRegistryPair.second(ecsManager->GetEntity(entityId));
            }
        }
        ImGui::EndPopup();
    }
};

void ExInspectorWindow::DrawAsset(AssetBrowserSelection* assetBrowserSelection){
    auto assetPath = assetBrowserSelection->GetPath();

    if(assetPath.has_extension())
    {
        auto assetExtension = assetPath.extension().string();
        if(assetExtension == ".png" || assetExtension == ".jpg" || assetExtension == ".jpeg")
        {
            spriteInformations.push_back(std::make_shared<SpriteInformation>(assetPath.filename(), assetPath.string(), glm::vec2(1,1)));
            auto spriteInformation = spriteInformations.back();

            auto textContent = "Image - " + assetPath.filename().string();
            auto textSize = ImGui::CalcTextSize(textContent.c_str());
            auto availableSize = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((availableSize / 2) - (textSize.x / 2));
            ImGui::Text("%s", textContent.c_str());

            auto size = spriteInformation->GetSpriteSize();
            auto difference = size.x - availableSize;

            if(difference > 0)
            {
                auto viewportGCD = ExMath::GetGCD(size.x, size.y);
                
                auto sizeW = ImGui::GetContentRegionAvail().x;
                auto sizeH = ImGui::GetContentRegionAvail().y;
                auto aspectW = size.x / viewportGCD;
                auto aspectH = size.y / viewportGCD;

                auto limitFactor = sizeW / sizeH;

                if(sizeW < size.x || sizeH < size.y)
                {
                    if(limitFactor >= 1){
                        size.x = sizeH;
                        size.x = (size.y / aspectH) * aspectW;
                    }
                    else{
                        size.x = sizeW;
                        size.y = (size.x / aspectW) * aspectH;
                    }
                }
            }

            ImGui::Image((ImTextureID)(spriteInformation->GetTexture()), {size.x, size.y}, {0,0}, {1,1}, {1,1,1,1}, {1,1,1,1});
            return;
        }

        if(assetExtension == ".exfile" || assetExtension == ".lua")
        {
            auto textContent = "File - " + assetPath.filename().string();
            auto textSize = ImGui::CalcTextSize(textContent.c_str());
            auto availableSize = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((availableSize / 2) - (textSize.x / 2));
            ImGui::Text("%s", textContent.c_str());

            ImGui::BeginChild((std::string("##") + assetPath.string()).c_str(), ImGui::GetContentRegionAvail(), ImGuiChildFlags_Borders);
            std::ifstream content(assetPath);
            std::stringstream contentBuffer;
            contentBuffer << content.rdbuf();

            ImGui::TextWrapped("%s", contentBuffer.str().c_str());

            ImGui::EndChild();

            return;
        }

        if(assetExtension == ".exworld")
        {
            auto textContent = "World - " + assetPath.filename().stem().string();
            auto textSize = ImGui::CalcTextSize(textContent.c_str());
            auto availableSize = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((availableSize / 2) - (textSize.x / 2));
            ImGui::Text("%s", textContent.c_str());

            float windowWidth = ImGui::GetWindowSize().x;
            float buttonWidth = ImGui::CalcTextSize("Open").x + ImGui::GetStyle().FramePadding.x * 2;
            float centerPos = (windowWidth - buttonWidth) * 0.5f;

            ImGui::SetCursorPosX(centerPos);
            if (ImGui::Button("Open"))
            {
                ECSWorldManager::LoadWorld(assetPath);
                EditorInterfaceGetters::worldWithoutPath = false;
            }

            buttonWidth = ImGui::CalcTextSize("Open as Incremental").x + ImGui::GetStyle().FramePadding.x * 2;
            centerPos = (windowWidth - buttonWidth) * 0.5f;

            ImGui::SetCursorPosX(centerPos);
            if (ImGui::Button("Open as Incremental"))
            {
                ECSWorldManager::LoadIncrementalWorld(assetPath);
                EditorInterfaceGetters::worldWithoutPath = false;
            }
            return;
        }

        ImGui::TextWrapped("%s", (assetExtension + " is not supported").c_str());

        return;
    }

    ImGui::Text("%s", assetPath.stem().c_str());
};