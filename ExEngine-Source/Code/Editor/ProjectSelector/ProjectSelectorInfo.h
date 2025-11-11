#pragma once
#include "../../Engine/JsonUtility/IJsonConvertable.h"
#include "ProjectInfo.h"
#include <vector>
#include <string>
#include <filesystem>

struct ProjectSelectorInfo: public IJsonConvertable{
public:
    std::vector<ProjectInfo> projectInfo;

    virtual nlohmann::json ToJson() override {
        nlohmann::json projectsJson = nlohmann::json::array();

        for (auto& p : projectInfo)
            projectsJson.push_back(p.ToJson());

        return {
            {"projectInfo", projectsJson}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("projectInfo") && json["projectInfo"].is_array()) {
            projectInfo.clear();

            for (auto& item : json["projectInfo"]) {
                ProjectInfo info;
                info.FromJson(item);
                projectInfo.push_back(info);
            }
        }
    }
};