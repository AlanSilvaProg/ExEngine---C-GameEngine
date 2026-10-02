#pragma once
#include <memory>
#include "RenderQueue/IRenderElement.h"
#include "../../ECS/ECSManager.h"
#include "../../ECSystems/CameraSystem/CameraSystem.h"
#include "../../ECSystems/RenderingSystem2D/RenderingSystem2D.h"
#include "../../ECSystems/UI/TextLabelSystem.h"
#include <memory>
#include <vector>

class ExRenderer{
private:
    static std::shared_ptr<ECSManager> ecsManager;
    static std::shared_ptr<RenderingSystem2D> renderingSystem2D;
    static std::shared_ptr<TextLabelSystem> textLabelSystem;
    static std::shared_ptr<ECSystemContext> preRenderSystemContext;

    static bool initialized;

    static void Render();
public:
    static void Initialize(std::shared_ptr<ECSManager> ecsManagerPtr);
    static void RenderSequence();
    static void RenderQueue();
    static const bool RenderOrderCheck(const LayerAttributes& a, const LayerAttributes& b);
    static std::shared_ptr<RenderingSystem2D> GetRenderingSystem2D();
    static std::shared_ptr<TextLabelSystem> GetTextLabelSystem();

    // Lets an embedder (e.g. an external editor) take exclusive control of camera selection and
    // rendering for a frame - disabling the PRE_RENDER context stops CameraSystem from running,
    // so it won't fight whatever's driving ExRendererGetters::currentRenderCameraTransform/
    // globalCameraZoom and push its own elements into the same render queue.
    static void SetGameplayCameraEnabled(bool enabled);

    static void Quit();
};