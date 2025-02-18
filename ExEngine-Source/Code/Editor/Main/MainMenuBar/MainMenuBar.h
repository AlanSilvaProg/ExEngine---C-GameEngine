#pragma once
#include "Section.h"
#include <memory>
#include <vector>

namespace ExEditor{

class MainMenuBar {
private:
    static std::vector<std::shared_ptr<Section>> sections;

public: 
    MainMenuBar();
    ~MainMenuBar();
    void Draw();

    static void AddSection(std::shared_ptr<Section> menuSection);
    static void RemoveSection(std::shared_ptr<Section> menuSection);
};

};