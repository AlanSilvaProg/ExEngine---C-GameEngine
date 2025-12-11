#pragma once
#include "../../../../../Engine/Core/ECS/ECSManager.h"
#include "../../../../../Engine/Core/Serializer/ExSerializedField.h"
#include "../../../../../Engine/Core/AssetManager/AssetManager.h"
#include "../../../../../Engine/Core/AssetManager/SpriteInformation.h"
#include "../EntityBrowser/EntityBrowserSelection.h"
#include "../AssetBrowser/AssetBrowserSelection.h"
#include "../../EditorWindow.h"
#include "../EngineConfig/WindowSizeManager.h"
#include <memory>
#include <vector>
#include <string>
#include <map>

class ExInspectorWindow : public EditorWindow{
private:
    std::shared_ptr<ECSManager> ecsManager;
    std::vector<std::shared_ptr<SpriteInformation>> spriteInformations;

    void DrawEntity(const EntityBrowserSelection* entityBrowserSelection);
    void DrawEntityComponent(const std::shared_ptr<IPool> componentPool, const int entityId);
    void DrawSerializedClass(const ExSerializedClass& serializeFields, const int id, bool root)const;
    void DrawComponentField(const ExSerializedField& exSerializedField, const std::string& className) const;
    void DrawAddComponentButton(const int entityId);

    void DrawAsset(const AssetBrowserSelection* assetBrowserSelection); 
public:
    ExInspectorWindow();
    void Draw(int phase) override; //0 == early 1 == late
};