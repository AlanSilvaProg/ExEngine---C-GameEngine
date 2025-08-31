#include "MainMenuBar.h"
#include "ProjectSection/ProjectSection.h"
#include "WindowSection/WindowSection.h"
#include "../../EditorEvents/EditorUpdateEventHandler.h"
#include "../../../Engine/Logger/Logger.h"
#include <imgui.h>

std::vector<std::shared_ptr<Section>> ExEditor::MainMenuBar::sections;

ExEditor::MainMenuBar::MainMenuBar(){
    AddSection(std::make_shared<ProjectSection>());
    AddSection(std::make_shared<WindowSection>());
    *EditorUpdateEventHandler::earlyHandler += [this](){ this->Draw(); };
}; 

ExEditor::MainMenuBar::~MainMenuBar(){
    sections.clear();
};

void ExEditor::MainMenuBar::Draw(){
    if (ImGui::BeginMainMenuBar())
    {
        for(auto section : sections){
            section->Draw();
        }
        ImGui::EndMainMenuBar();
    }
};

void ExEditor::MainMenuBar::AddSection(std::shared_ptr<Section> menuSection){
    for(auto s : sections){
        if(s == menuSection)
            return;
    }
    sections.push_back(menuSection);
};

void ExEditor::MainMenuBar::RemoveSection(std::shared_ptr<Section> menuSection){
    sections.erase(std::remove_if(sections.begin(), sections.end(), [menuSection](std::shared_ptr<Section> section) { 
        return section == menuSection; }), sections.end());
};