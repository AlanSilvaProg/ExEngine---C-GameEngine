#include "ProjectSection.h"
#include "../../../../Engine/Core/Runtime/App.h"
#include "../../../../Engine/Logger/Logger.h"
#include "../../../../Engine/Core/Engine.h"
#include "../../../../Engine/Core/Scene/ECSWorldManager.h"
#include "../../ProjectManager/ProjectManager.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include <imgui.h>
#include <filesystem>

void ProjectSection::Draw(){
    if (ImGui::BeginMenu("Project"))
    {
#ifdef EXENGINE_MACOS
        if (ImGui::MenuItem("Save...", "CMD+S")) { EditorInterfaceGetters::Save(); }
#else
        if (ImGui::MenuItem("Save...", "CTRL+S")) { EditorInterfaceGetters::Save(); }
#endif

        if(ImGui::MenuItem("Create..."))
        {
            creatingProject = true;
        }

        if(ImGui::MenuItem("Open..."))
        {
            std::filesystem::path enginePath = Engine::GetEnginePath();
            const char* folder = tinyfd_selectFolderDialog(
                "Select project folder",
                enginePath.c_str()
            );

            if(folder != nullptr)
            {
                auto targetPath = std::filesystem::path(folder);
                if(ProjectManager::IsValidProject(targetPath))
                {
                    ProjectManager::TryOpenProject({ProjectManager::GetProjectName(targetPath), targetPath.string()});
                }
            }
        }

        if(ImGui::MenuItem("Build"))
        {
            EditorInterfaceGetters::buildWindowEnabled = true;
        }

        if(ImGui::MenuItem("Project Settings"))
        {
            EditorInterfaceGetters::projectSettingsEnabled = !EditorInterfaceGetters::projectSettingsEnabled;
        }

        if(ImGui::SmallButton(App::isPlaying ? "Stop" : "Play"))
        {
            App::isPlaying = !App::isPlaying;

            if(!App::isPlaying) 
            {
                EditorInterfaceGetters::Reload();
            }
        }

        ImGui::EndMenu();
    }

    if(creatingProject)
    {
        static bool projectCreatingVisibility = true;
        if(ImGui::Begin("Create project", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize)){
            static char projectName[128] = "";
            ImGui::InputTextWithHint("Project name", "Enter name here", projectName, IM_ARRAYSIZE(projectName));

            ImVec2 sz = ImVec2(-FLT_MIN, 0.0f);
            if(ImGui::Button("Create", sz)){
                auto success = ProjectManager::CreateNewProject(projectName);
                creatingProject = false;
            };

            if(!ImGui::IsWindowFocused()) creatingProject = false;
        }
        
        ImGui::End();
        
        if(!projectCreatingVisibility) creatingProject = false;
    }
};

void ProjectSection::OpenProject(const ProjectInfo& projectInformation) const{

};