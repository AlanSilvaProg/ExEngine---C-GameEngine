#include "ProjectManager.h"
#include "../../../Engine/File/FileManagement.h"
#include "../../../Engine/Logger/Logger.h"
#include "../../../Engine/Core/Engine.h"
#include <string>
#include <filesystem>
#include <imgui.h>

ProjectInfo ProjectManager::currentProject;

//ToDo create new project setting a name for it
bool ProjectManager::CreateNewProject(const std::string& projectName){
    std::filesystem::path path = "";
    auto projName = projectName.size() == 0 ? std::string("NewProject") : projectName;

    std::string p = Engine::GetEnginePath() / std::string("Projects/" + projName + "/Assets/Scenes/StandardScene");
    path.append(p);

    if(std::filesystem::exists(path) && std::filesystem::is_directory(path))
        return false;

    //ToDo Need a base project with camera
    std::string value = std::string("");
    path = path.replace_extension(".exworld");
    FileManagement::SaveFile(path, value);
    return true;
};

//ToDo Open system dialog to open a folder
bool ProjectManager::TryOpenProject(const ProjectInfo& projectName){
    Logger::Log("Will open the project at folder: " + projectName.projectPath + " with the name: " + projectName.projectName.c_str());
    return false;
};

bool ProjectManager::CreateBaseProjectAt(const std::filesystem::path path, std::string name, ProjectInfo& projectInfo){
    std::filesystem::path finalPath = path / name / "ExProject";
    finalPath.replace_extension(".exproj");

    //ToDo create a base shape with setup informations and validate whenever it get opened

    projectInfo = ProjectInfo(name, path);
    auto result = FileManagement::SaveFile(finalPath, projectInfo.ToJson().dump());

    if(result) 
        Logger::Log("New project created at: " + (path / name).string());

    return result;
};