#pragma once
#include "../Section.h"
#include "../../../../Engine/File/FileManagement.h"
#include "../../ProjectManager/ProjectInfo.h"
#include "../../EditorInterfaceGetters.h"

class ProjectSection : public Section {
private:
    bool creatingProject;
    void OpenProject(const ProjectInfo& projectInformation) const;
public:
    ProjectSection() = default;
    ~ProjectSection() = default;

    void virtual Draw() override;
};