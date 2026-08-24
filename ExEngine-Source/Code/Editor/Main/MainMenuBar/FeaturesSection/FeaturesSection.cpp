#include "FeaturesSection.h"
#include "../../EditorInterfaceGetters.h"
#include "../../../../Engine/Core/Runtime/Settings/RuntimeSettings.h"
#include "../../../../Engine/Core/Configuration/ConfigurationFileManager.h"
#include "../../../../Engine/Core/CollisionSystem/ExPhysicsEngine.h"
#include <imgui.h>

void FeaturesSection::Draw(){
    if(ImGui::BeginMenu("Features"))
    {
        bool physicsEnabled = RuntimeSettings::GetPhysicsEnabled();
        if(ImGui::MenuItem("Enable Physics Engine", nullptr, &physicsEnabled))
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
        if(ImGui::MenuItem("Draw all Physics", nullptr, &drawAllPhysics))
        {
            RuntimeSettings::SetDrawAllPhysics(drawAllPhysics);
            ConfigurationFileManager::SaveCurrentState();
        }

        ImGui::EndMenu();
    }
};
