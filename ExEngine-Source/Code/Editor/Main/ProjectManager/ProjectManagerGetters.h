#pragma once
#include "string"
#include <filesystem>

class ProjectManagerGetters {
public:
    static bool projectOpened;
    static std::string currentProjectPath;
};