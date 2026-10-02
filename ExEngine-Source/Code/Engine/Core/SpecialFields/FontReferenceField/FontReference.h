#pragma once
#include <filesystem>
#include <string>
#include "../../../JsonUtility/IJsonConvertable.h"

struct FontReference : public IJsonConvertable{
public:
    std::string id;
    std::filesystem::path path;

    FontReference() = default;
    FontReference(const std::string& id, const std::filesystem::path& path) : id(std::move(id)), path(std::move(path)) {};

    virtual nlohmann::json ToJson() override {
        return {
            {"id", id},
            {"path", path}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("id"))
            id = json["id"].get<std::string>();

        if (json.contains("path"))
            path = json["path"].get<std::string>();
    }
};
