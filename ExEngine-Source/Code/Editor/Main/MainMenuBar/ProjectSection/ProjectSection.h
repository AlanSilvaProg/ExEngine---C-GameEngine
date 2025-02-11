#include "../Section.h"

class ProjectSection : public Section {
private:
    void Save();
    void Load();
public:
    ProjectSection() = default;
    ~ProjectSection() = default;

    void virtual Draw() override;
};