#pragma once
#include <string>
#include <filesystem>

struct ProjectInformation{
public:
    std::string name;
    std::filesystem::path path;

    ProjectInformation(std::string name, std::filesystem::path path) : name(name), path(path) {};
};