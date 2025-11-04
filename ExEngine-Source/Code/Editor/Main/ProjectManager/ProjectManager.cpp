#include "ProjectManager.h"
#include "../../../Engine/File/FileManagement.h"
#include "../../../Engine/Logger/Logger.h"
#include "../../../Engine/Core/Engine.h"
#include <string>
#include <filesystem>
#include <imgui.h>

ProjectInformation* ProjectManager::currentProject = nullptr;
std::vector<ProjectInformation> ProjectManager::projectInformation;

//ToDo create new project setting a name for it
bool ProjectManager::CreateNewProject(const std::string& projectName){
    std::filesystem::path path = "";
    auto projName = projectName.size() == 0 ? std::string("NewProject") : projectName;

    std::string p = Engine::GetEnginePath() / std::string("Projects/" + projName + "/Assets/Scenes/StandardScene");
    path.append(p);

    if(std::filesystem::exists(path) && std::filesystem::is_directory(path))
        return false;

    //ToDo Need a base project
    std::string value = std::string("Aqui jás uma cena");
    FileManagement::SaveFile(path, value);
    return true;
};

//ToDo Open system dialog to open a folder
bool ProjectManager::TryOpenProject(const ProjectInformation& projectName){
    Logger::Log("Will open the project at folder: " + projectName.path.string() + " with the name: " + projectName.name.c_str());
    return false;
};

std::vector<ProjectInformation>& ProjectManager::GetProjectList(){
    std::filesystem::path path = "";
    std::string p = Engine::GetEnginePath() / std::string("Projects/");
    path.append(p);

    if(!std::filesystem::exists(path))
    {
        projectInformation.clear();
    }
    else
    {
        for (const auto & entry : std::filesystem::directory_iterator(path))
        {
            std::filesystem::path entryPath = entry.path();
            auto directoryExist = std::filesystem::is_directory(entryPath);

            if(directoryExist)
            {
                auto last_folder = entryPath.filename().empty() ? entryPath.parent_path().filename().string() : entryPath.filename().string();
                
                auto permissionToStore = true;
                for(const auto& information : projectInformation)
                {
                    if(information.name == last_folder)
                    {
                        permissionToStore = false;
                        
                        if(!directoryExist) 
                        {
                            if(currentProject->path.compare(information.path)) currentProject = nullptr;

                            projectInformation.erase(std::remove_if(projectInformation.begin(), projectInformation.end(), [information](ProjectInformation& pInformation){
                                return information.path.compare(pInformation.path);
                            }));
                        }
                    }
                }
                if(!permissionToStore) continue;

                ProjectInformation projInformation(last_folder, entryPath);
                projectInformation.push_back(projInformation);
            }
        }
    }

    return projectInformation;
};