#pragma once
#include "Algorithms/JsonExtensions.h"
#include "../../JsonUtility/IJsonConvertable.h"
#include <glm/glm.hpp>

struct ExRect : public IJsonConvertable{
public:
    glm::vec2 beginRect;
    glm::vec2 endRect;

    ExRect() = default;
    ExRect(glm::vec2 begin, glm::vec2 end) : beginRect(begin), endRect(end) { };
    ~ExRect() = default;

    virtual nlohmann::json ToJson() override{
        return {
            {"beginRect", JsonExtensions::glm_to_json(beginRect)},
            {"endRect", JsonExtensions::glm_to_json(endRect)}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("beginRect")) JsonExtensions::glm_from_json(json["beginRect"], beginRect);
        if (json.contains("endRect")) JsonExtensions::glm_from_json(json["endRect"], endRect);
    };
};