#include "ExProjectSettingsWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Configuration/ConfigurationFileManager.h"
#include "../../../../../Engine/Core/Runtime/Settings/RuntimeSettings.h"
#include "../../../../../Engine/Core/CollisionSystem/ExPhysicsEngine.h"
#include "../EngineConfig/WindowSizeManager.h"
#include <imgui.h>
#include <string>

void ExProjectSettingsWindow::Draw(int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::projectSettingsEnabled) return;

    // Apply minimum size constraint and validate initial size using WindowSizeManager
    WindowSizeManager::ApplyConstraintWithValidatedSize("Project Settings", ImVec2(600, 400), ImGuiCond_Once);

    if(ImGui::Begin("Project Settings", &EditorInterfaceGetters::projectSettingsEnabled))
    {
        static bool lastFrameHasEdition = false;
        bool editingSomething = false;
        bool activingSomething = false;

        auto targetFps = RuntimeSettings::GetTargetFps();
        ImGui::DragInt("FPS Limit", &targetFps, 1.0, 1.0, 320);
        if(ImGui::IsItemActive())
        {
            activingSomething = true;
            if(ImGui::IsItemEdited())
            {
                editingSomething = lastFrameHasEdition = true;
                RuntimeSettings::SetTargetFps(targetFps);
            }
        }

        if(!editingSomething && !activingSomething && lastFrameHasEdition)
        {
            lastFrameHasEdition = false;
            ConfigurationFileManager::SaveCurrentState();
        }

        ImGui::Separator();
        ImGui::Text("Physics");

        bool physicsEnabled = RuntimeSettings::GetPhysicsEnabled();
        if(ImGui::Checkbox("Enable Physics Engine", &physicsEnabled))
        {
            RuntimeSettings::SetPhysicsEnabled(physicsEnabled);
            ConfigurationFileManager::SaveCurrentState();

            if(physicsEnabled)
            {
                ExPhysicsEngine::Initialize(EditorInterfaceGetters::engine->GetECSManagerPtr());
            }
            else
            {
                ExPhysicsEngine::Stop();
            }
        }

        bool drawAllPhysics = RuntimeSettings::GetDrawAllPhysics();
        if(ImGui::Checkbox("Draw all Physics", &drawAllPhysics))
        {
            RuntimeSettings::SetDrawAllPhysics(drawAllPhysics);
            ConfigurationFileManager::SaveCurrentState();
        }
    }
    ImGui::End();
};