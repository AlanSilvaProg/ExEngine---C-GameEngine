#pragma once
#include "../../JsonUtility/IJsonConvertable.h"
#include <filesystem>
#include <string>

struct ProjectInfo : public IJsonConvertable{
public:
    std::string projectName;
    std::string projectPath;
    std::string defaultWorld; // .exworld stem, under <project>/Assets/Worlds, loaded automatically when a shipped (non-editor) build starts
    bool devMode = true; // Build window: on keeps logs + debug symbols, off is a stripped release build ready to distribute

    ProjectInfo() = default;
    ProjectInfo(std::string name, std::string path) : projectName(name), projectPath(path){};
    ~ProjectInfo() = default;

    inline const std::filesystem::path GetFullPath() const { return std::filesystem::path(projectPath) / projectName; };

    virtual nlohmann::json ToJson() override {
        return {
            {"projectName", projectName},
            {"projectPath", projectPath},
            {"defaultWorld", defaultWorld},
            {"devMode", devMode}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        projectName = json.value("projectName", "");
        projectPath = json.value("projectPath", "");
        defaultWorld = json.value("defaultWorld", "");
        devMode = json.value("devMode", true);
    }
};
