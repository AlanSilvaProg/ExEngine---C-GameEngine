#pragma once
#include "../../EditorWindow.h"
#include "../../../../../Engine/Core/Engine.h"
#include "../../../../../Engine/Core/ECS/ECSManager.h"
#include <memory>

class ECSAdmin: public EditorWindow{
private:
    std::shared_ptr<ECSManager> ecsManager;

    bool showWorldEntities;
    bool showSystems;
    bool includeInternals;
public:
    ECSAdmin();

    void Draw(int phase) override; //0 == early 1 == late
};