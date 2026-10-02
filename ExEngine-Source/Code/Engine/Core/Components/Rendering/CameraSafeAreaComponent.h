#pragma once
#include <glm/glm.hpp>
#include "../../ECS/ECSManager.h"
#include "../../ECS/InternalRegistry/ComponentRegistry.h"
#include "../../Utils/Algorithms/JsonExtensions.h"

struct CameraSafeAreaComponent : public EComponentS<CameraSafeAreaComponent>{
public:
    static constexpr unsigned int ComponentId = 6;
    static constexpr const char* ComponentGroup = "Rendering";

    glm::vec2 positionBegin = glm::vec2(-100, -100);
    glm::vec2 positionEnd = glm::vec2(100, 100);
    glm::vec2 limitBegin = glm::vec2(-200, -200);
    glm::vec2 limitEnd = glm::vec2(200, 200);
    bool showGizmo = true;

    CameraSafeAreaComponent() = default;
    CameraSafeAreaComponent(glm::vec2 positionBegin, glm::vec2 positionEnd) : positionBegin(positionBegin), positionEnd(positionEnd) {};
    CameraSafeAreaComponent(const CameraSafeAreaComponent& safeArea){
        positionBegin = safeArea.positionBegin;
        positionEnd = safeArea.positionEnd;
        limitBegin = safeArea.limitBegin;
        limitEnd = safeArea.limitEnd;
        showGizmo = safeArea.showGizmo;
    };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), positionBegin, true),
        EX_SERIALIZER((*this), positionEnd, true),
        EX_SERIALIZER((*this), limitBegin, true),
        EX_SERIALIZER((*this), limitEnd, true),
        EX_SERIALIZER((*this), showGizmo, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"positionBegin", JsonExtensions::glm_to_json(positionBegin)},
            {"positionEnd", JsonExtensions::glm_to_json(positionEnd)},
            {"limitBegin", JsonExtensions::glm_to_json(limitBegin)},
            {"limitEnd", JsonExtensions::glm_to_json(limitEnd)},
            {"showGizmo", showGizmo}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("positionBegin")) JsonExtensions::glm_from_json(json["positionBegin"], positionBegin);
        if (json.contains("positionEnd")) JsonExtensions::glm_from_json(json["positionEnd"], positionEnd);
        if (json.contains("limitBegin")) JsonExtensions::glm_from_json(json["limitBegin"], limitBegin);
        if (json.contains("limitEnd")) JsonExtensions::glm_from_json(json["limitEnd"], limitEnd);
        if (json.contains("showGizmo")) showGizmo = json["showGizmo"].get<bool>();
    }
};

REGISTER_COMPONENT(CameraSafeAreaComponent)