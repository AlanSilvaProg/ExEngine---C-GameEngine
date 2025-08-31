#pragma once
#include <memory>
#include "../../ECS/ECSManager.h"
#include "../../CameraSystem/CameraSystem.h"
#include "../Systems/RenderingSystem2D.h"

class ExRenderer{
private:
    static std::shared_ptr<ECSManager> ecsManager;
    static std::shared_ptr<RenderingSystem2D> renderingSystem2D;
    static std::shared_ptr<CameraSystem> cameraSystem;

    static bool initialized;

    static void Render();
public:
    static void Initialize(std::shared_ptr<ECSManager> ecsManagerPtr);
    static void RenderSequence();

    static std::shared_ptr<RenderingSystem2D> GetRenderingSystem2D();

    static void Quit();
};