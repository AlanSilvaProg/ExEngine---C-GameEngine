#include "ProjectSection.h"
#include "../../../../Engine/Core/Input/Input.h"
#include "../../../../Engine/Core/Runtime/App.h"
#include "../../../../Engine/Logger/Logger.h"
#include "../../EditorInterfaceGetters.h"
#include <imgui.h>

void ProjectSection::Draw(){
    if (ImGui::BeginMenu("Project"))
    {
#ifdef EXENGINE_MACOS
        if (ImGui::MenuItem("Save...", "CMD+S")) { Save(); }
#else
        if (ImGui::MenuItem("Save...", "CTRL+S")) { Save(); }
#endif

        if(ImGui::MenuItem("Create..."))
        {
            creatingProject = true;
        }

        if(ImGui::BeginMenu("Open..."))
        {
            auto projectId = 0;
            auto& projectList = ProjectManager::GetProjectList();
            for(auto& project : projectList)
            {
                ImGui::PushID(projectId);
                if(ImGui::SmallButton(project.name.c_str()))
                {
                    ProjectManager::TryOpenProject(project);
                }
                ImGui::PopID();
                projectId++;
            }

            ImGui::EndMenu();
        }

        if(ImGui::MenuItem("Project Settings"))
        {
            EditorInterfaceGetters::projectSettingsEnabled = !EditorInterfaceGetters::projectSettingsEnabled;
        }

        if((Input::GetButtonDown(SDLK_LCTRL) || Input::GetButtonDown(SDLK_RCTRL)) && Input::GetButtonDown(SDLK_s))
        {
            Save();
        }

        if(ImGui::SmallButton(App::isPlaying ? "Stop" : "Play"))
        {
            App::isPlaying = !App::isPlaying;

            if(!App::isPlaying) 
            {
                Load();
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

            ImGui::End();
        }
        if(!projectCreatingVisibility) creatingProject = false;
    }
};

void ProjectSection::OpenProject(const ProjectInformation& projectInformation) const{

};

void ProjectSection::Save(){

};
    
void ProjectSection::Load(){

};