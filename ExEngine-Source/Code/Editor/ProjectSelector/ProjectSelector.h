#pragma once 
#include "ProjectSelectorInfo.h"
#include "../../Engine/Core/Engine.h"
#include <string>
#include <SDL2/SDL.h>

#ifndef PROJECT_SELECTOR_PATH
#define PROJECT_SELECTOR_PATH Engine::GetEnginePath()/"Library/ProjectSelector/"
#endif

class ProjectSelector{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    ProjectSelectorInfo projectSelectorInfo;

    //Project creation
    void CreateProjectSelectorWindow();
    void CreateProjectSelectorEditor();

    void SaveProjectInfo();

    //Lifecycle
    void RenderEditor();
    bool CloseByInput() const;
public:
    inline ProjectSelector(){ CreateProjectSelectorWindow(); CreateProjectSelectorEditor(); };
    ~ProjectSelector();

    std::string Run();
};