#include "ExInspectorWindow.h"
#include "../ElementSelectionController.h"
#include "../WindowsUtility/ElementTypeId.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../Utils/FileSystemOpener.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Serializer/ISerializable.h"
#include "../../../../../Engine/Core/Rendering/Layer/LayerAttributes.h"
#include "../../../../../Engine/Core/ECS/Component/EComponentS.h"
#include "../../../../../Engine/Core/Components/TransformComponent.h"
#include "../../../../../Engine/Core/ECS/InternalRegistry/ComponentRegistry.h"
#include "../../../../../Engine/Core/Utils/Algorithms/ExMath.h"
#include "../../../../../Engine/Core/Scene/ECSWorldManager.h"
#include "../../../../../Engine/Core/SpecialFields/SpriteReferenceField/SpriteReference.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <SDL.h>

ExInspectorWindow::ExInspectorWindow(){
    ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    assetManager = AssetManager::GetInstance();
    showSaveConfirmDialog = false;
    lastSelectedAssetPath = "";
    pendingSelectionPath = "";
};

void ExInspectorWindow::Draw(int phase){
    if(phase != 1) return;

    spriteInformations.clear();

    // Apply minimum size constraint using WindowSizeManager
    WindowSizeManager::ApplyMinimumSizeConstraint("ExInspector");
    
    // Draw save confirmation dialog if needed
    if (showSaveConfirmDialog) {
        DrawSaveConfirmDialog();
    }
    
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
                auto assetSelection = dynamic_cast<AssetBrowserSelection*>(selectedElement);
                std::string currentAssetPath = assetSelection->GetPath().string();
                
                // Check for selection change and unsaved changes
                if (currentAssetPath != lastSelectedAssetPath) {
                    CheckForUnsavedChanges(currentAssetPath);
                }
                
                if (!showSaveConfirmDialog) {
                    DrawAsset(assetSelection);
                }
            }
        }
        else {
            if (!lastSelectedAssetPath.empty()) {
                CheckForUnsavedChanges("");
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
    + std::to_string(entity->GetId()) + "_Inspector";
    ImGui::PushID(uniqueId.c_str());

    static const char* entityNameLabel = "";
    char buf[256]{};
    std::snprintf(buf, sizeof(buf), "%s", entity->GetName().c_str());
    if (ImGui::InputText(entityNameLabel, buf, sizeof(buf))) {
        entity->ChangeName(buf);
    }

    ImGui::PopID();

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

    const auto componentId = component->GetComponentId();
    if(ComponentRegistry::components.find(componentId) == ComponentRegistry::components.end())
    {
        DrawMissingComponent(entityId, componentId);
        return;
    }

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

void ExInspectorWindow::DrawMissingComponent(const int entityId, const int componentId){
    ImGui::BeginGroup();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::Text("Missing Component (Id: %d)", componentId);
    ImGui::PopStyleColor();
    ImGui::TextWrapped("The script that defined this component no longer exists.");

    ImGui::PushID(("missing_component_" + std::to_string(componentId)).c_str());
    if(ImGui::SmallButton("Remove Component"))
    {
        ecsManager->GetEntity(entityId)->RemoveComponent(componentId);
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
    else if (exSerializedField.fieldType == typeid(SpriteReference)){
        auto spriteReference = static_cast<SpriteReference*>(exSerializedField.field_ptr);
        bool spriteReferenceBtn = false;
        auto buttonSize = ImVec2(150,150);
        
        if(std::filesystem::exists(spriteReference->path)) {
            auto texture = assetManager->GetTextureAsset(spriteReference->id, spriteReference->path);
        
            ImTextureID textureId = (ImTextureID)(intptr_t)texture;
            spriteReferenceBtn = ImGui::ImageButton(spriteReference->id.c_str(), textureId, buttonSize);

            assetManager->FreeAsset(spriteReference->id);
        }
        else{
            spriteReferenceBtn = ImGui::Button("empty ( Drag or Select file )", buttonSize);
        }

        if(ImGui::BeginDragDropTarget()){
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::to_string(ElementTypeId::SPRITE).c_str())){
                auto spritePayload = *(const nlohmann::json*)payload->Data;
                spriteReference->FromJson(spritePayload);

                if(exSerializedField.onFieldChanged)
                    exSerializedField.onFieldChanged();
            }
            ImGui::EndDragDropTarget();
        }
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

        if(assetExtension == ".hpp")
        {
            DrawHppFileEditor(assetPath);
            return;
        }

        if(assetExtension == ".exfile")
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

void ExInspectorWindow::DrawHppFileEditor(const std::filesystem::path& assetPath) {
    std::string pathStr = assetPath.string();
    
    // Header with double-click to open in IDE
    auto textContent = "Hpp Script - " + assetPath.filename().string();
    auto textSize = ImGui::CalcTextSize(textContent.c_str());
    auto availableSize = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX((availableSize / 2) - (textSize.x / 2));
    ImGui::Text("%s", textContent.c_str());
    
    // Double-click on header to open in IDE
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Double-click to open in system editor");
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            FileSystemOpener::OpenFileInSystemEditor(assetPath);
        }
    }
    
    // Load file content if not already loaded
    if (hppFileContents.find(pathStr) == hppFileContents.end()) {
        std::ifstream file(assetPath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            hppFileContents[pathStr] = buffer.str();
            originalHppContents[pathStr] = hppFileContents[pathStr];
            hppFileModified[pathStr] = false;
            file.close();
        } else {
            hppFileContents[pathStr] = "";
            originalHppContents[pathStr] = "";
            hppFileModified[pathStr] = false;
        }
    }

    // Buttons row
    ImGui::Spacing();

    if (ImGui::Button("Open..")) {
        FileSystemOpener::OpenFileInSystemEditor(assetPath);
    }

    // Apply button (only show if modified)
    if (hppFileModified[pathStr]) {
        ImGui::SameLine();

        if (ImGui::Button("Apply")) {
            std::ofstream file(assetPath);
            if (file.is_open()) {
                file << hppFileContents[pathStr];
                file.close();
                originalHppContents[pathStr] = hppFileContents[pathStr];
                hppFileModified[pathStr] = false;
                Logger::Log("Saved changes to: " + pathStr);
            } else {
                Logger::Log("Failed to save file: " + pathStr);
            }
        }
    }

    ImGui::Spacing();

    // Text editor
    ImGui::BeginChild((std::string("##HppEditor") + pathStr).c_str(),
                      ImVec2(0, ImGui::GetContentRegionAvail().y),
                      ImGuiChildFlags_Borders);

    // Create a large text buffer for editing
    static char textBuffer[32768]; // 32KB buffer

    // Copy current content to buffer if it fits
    std::string& currentContent = hppFileContents[pathStr];
    if (currentContent.length() < sizeof(textBuffer) - 1) {
        std::strncpy(textBuffer, currentContent.c_str(), sizeof(textBuffer) - 1);
        textBuffer[sizeof(textBuffer) - 1] = '\0';
    }

    // Multi-line text input
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    if (ImGui::InputTextMultiline("##HppContent", textBuffer, sizeof(textBuffer),
                                  ImVec2(-1, -1), flags)) {
        std::string newContent(textBuffer);
        if (newContent != currentContent) {
            hppFileContents[pathStr] = newContent;
            hppFileModified[pathStr] = (newContent != originalHppContents[pathStr]);
        }
    }
    
    // Double-click on text editor to open in IDE
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Double-click to open in system editor");
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            FileSystemOpener::OpenFileInSystemEditor(assetPath);
        }
    }
    
    ImGui::EndChild();
};

void ExInspectorWindow::CheckForUnsavedChanges(const std::string& newAssetPath) {
    // Check if we have unsaved changes in the current Hpp file
    if (!lastSelectedAssetPath.empty() &&
        lastSelectedAssetPath.ends_with(".hpp") &&
        hppFileModified.find(lastSelectedAssetPath) != hppFileModified.end() &&
        hppFileModified[lastSelectedAssetPath]) {
        
        // We have unsaved changes, show confirmation dialog
        showSaveConfirmDialog = true;
        pendingSelectionPath = newAssetPath;
    } else {
        // No unsaved changes, proceed with selection change
        lastSelectedAssetPath = newAssetPath;
    }
}

void ExInspectorWindow::DrawSaveConfirmDialog() {
    ImGui::OpenPopup("Unsaved Changes");
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have unsaved changes in:");
        ImGui::Text("%s", std::filesystem::path(lastSelectedAssetPath).filename().string().c_str());
        ImGui::Separator();
        ImGui::Text("Do you want to save your changes?");
        
        ImGui::Spacing();
        
        // Save button
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            std::ofstream file(lastSelectedAssetPath);
            if (file.is_open()) {
                file << hppFileContents[lastSelectedAssetPath];
                file.close();
                originalHppContents[lastSelectedAssetPath] = hppFileContents[lastSelectedAssetPath];
                hppFileModified[lastSelectedAssetPath] = false;
                Logger::Log("Saved changes to: " + lastSelectedAssetPath);
            } else {
                Logger::Log("Failed to save file: " + lastSelectedAssetPath);
            }

            lastSelectedAssetPath = pendingSelectionPath;
            showSaveConfirmDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // Don't Save button
        if (ImGui::Button("Don't Save", ImVec2(120, 0))) {
            // Discard changes
            if (hppFileContents.find(lastSelectedAssetPath) != hppFileContents.end()) {
                hppFileContents[lastSelectedAssetPath] = originalHppContents[lastSelectedAssetPath];
                hppFileModified[lastSelectedAssetPath] = false;
            }
            
            lastSelectedAssetPath = pendingSelectionPath;
            showSaveConfirmDialog = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

bool ExInspectorWindow::HasUnsavedHppChanges() const {
    for (const auto& pair : hppFileModified) {
        if (pair.second) {
            return true;
        }
    }
    return false;
};



