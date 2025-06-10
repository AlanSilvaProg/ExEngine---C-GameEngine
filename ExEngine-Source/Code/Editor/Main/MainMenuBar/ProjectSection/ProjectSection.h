#pragma once
#include "../Section.h"
#include "../../../../Engine/File/FileManagement.h"
#include "../../ProjectManager/ProjectInformation.h"
#include "../../ProjectManager/ProjectManager.h"

class ProjectSection : public Section {
private:
    bool creatingProject;
    void OpenProject(const ProjectInformation& projectInformation) const;
    void Save();
    void Load();
public:
    ProjectSection() = default;
    ~ProjectSection() = default;

    void virtual Draw() override;
};