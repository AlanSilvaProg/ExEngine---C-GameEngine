#pragma once
#include "../../../Engine/JsonUtility/IJsonConvertable.h"
#include <filesystem>
#include <string>

struct ProjectInfo : public IJsonConvertable{
public:
    std::string projectName;
    std::string projectPath;

    ProjectInfo() = default;
    ProjectInfo(std::string name, std::string path) : projectName(name), projectPath(path){};
    ~ProjectInfo() = default;

    inline const std::filesystem::path GetFullPath() const { return std::filesystem::path(projectPath) / projectName; };

    virtual nlohmann::json ToJson() override {
        return {
            {"projectName", projectName},
            {"projectPath", projectPath}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        projectName = json.value("projectName", "");
        projectPath = json.value("projectPath", "");
    }
};