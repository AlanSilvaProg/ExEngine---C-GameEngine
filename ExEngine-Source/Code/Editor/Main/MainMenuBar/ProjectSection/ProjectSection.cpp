#include "ProjectSection.h"
#include "../../../../Engine/Core/Input/Input.h"
#include <imgui.h>

#include "../../../../Engine/Core/Runtime/App.h"

void ProjectSection::Draw(){
    if (ImGui::BeginMenu("Project"))
    {
#ifdef EXENGINE_MACOS
        if (ImGui::MenuItem("Save...", "CMD+S")) { Save(); }
#else
        if (ImGui::MenuItem("Save...", "CTRL+S")) { Save(); }
#endif

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
};

void ProjectSection::Save(){

};
    
void ProjectSection::Load(){

};