#pragma once
#include "../../EditorWindow.h"
#include "../EntityBrowser/EntityBrowserSelection.h"
#include "../../../../../Engine/Core/ECS/ECSManager.h"
#include "../../../../../Engine/Core/Serializer/ExSerializedField.h"
#include <memory>
#include <vector>

class ExInspectorWindow : public EditorWindow{
private:
    std::shared_ptr<ECSManager> ecsManager;

    void DrawEntity(EntityBrowserSelection* entityBrowserSelection);
    void DrawEntityComponent(const std::shared_ptr<IPool> componentPool, const int entityId);
    void DrawSerializedClass(const ExSerializedClass& serializeFields, const int id, bool root)const;
    void DrawComponentField(const ExSerializedField& exSerializedField, const std::string& className) const;

    //void DrawAsset(); 
public:
    ExInspectorWindow();
    void Draw(int phase) override; //0 == early 1 == late
};