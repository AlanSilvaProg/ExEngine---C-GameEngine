#pragma once
#include "../../EditorWindow.h"
#include "../../../../../Engine/Core/Engine.h"
#include "../../../../../Engine/Core/ECS/ECSManager.h"
#include <memory>

class ECSMonitoring: public EditorWindow{
private:
    std::shared_ptr<Engine> enginePtr;
    std::shared_ptr<ECSManager> ecsManagerPtr;
public:
    ECSMonitoring();

    void Draw(int phase) override; //0 == early 1 == late
};