#pragma once 
#include "ProjectSelectorInfo.h"
#include <string>
#include <SDL2/SDL.h>

#ifndef PROJECT_SELECTOR_PATH
#define PROJECT_SELECTOR_PATH "Library/ProjectSelector/"
#endif

class ProjectSelector{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    ProjectSelectorInfo projectSelectorInfo;

    //Project creation
    void CreateProjectSelectorWindow();
    void CreateProjectSelectorEditor();

    //Project helpers
    inline bool IsValidProject(const std::filesystem::path path) const { return std::filesystem::exists(path / "ExProject.exproj"); };
    std::string GetProjectName(const std::filesystem::path path) const { return path.stem(); };
    bool CreateBaseProjectAt(const std::filesystem::path path, std::string name);
    void SaveProjectInfo();

    //Lifecycle
    void RenderEditor();
    bool CloseByInput() const;
public:
    inline ProjectSelector(){ CreateProjectSelectorWindow(); CreateProjectSelectorEditor(); };
    ~ProjectSelector();

    std::string Run();
};