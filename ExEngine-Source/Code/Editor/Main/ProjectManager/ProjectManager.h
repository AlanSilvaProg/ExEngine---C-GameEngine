#pragma once
#include "ProjectInfo.h"
#include <string>
#include <vector>
#include <memory>

class ProjectManager{
private:
    static ProjectInfo currentProject;
public:
    static bool CreateNewProject(const std::string& projectName);
    static bool TryOpenProject(const ProjectInfo& project);

    static bool CreateBaseProjectAt(const std::filesystem::path path, std::string name, ProjectInfo& projectInfo);
    static inline bool IsValidProject(const std::filesystem::path path) { return std::filesystem::exists(path / "ExProject.exproj"); };
    static std::string GetProjectName(const std::filesystem::path path) { return path.parent_path().filename(); };
};
