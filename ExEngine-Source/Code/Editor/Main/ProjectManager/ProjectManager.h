#pragma once
#include "ProjectInformation.h"
#include <string>
#include <vector>
#include <memory>

class ProjectManager{
private:
    static ProjectInformation* currentProject;
    static std::vector<ProjectInformation> projectInformation; // project list
public:
    static bool CreateNewProject(const std::string& projectName);
    static bool TryOpenProject(const ProjectInformation& project);
    static std::vector<ProjectInformation>& GetProjectList();
};
