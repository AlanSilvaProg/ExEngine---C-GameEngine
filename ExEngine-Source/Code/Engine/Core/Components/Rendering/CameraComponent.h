#pragma once
#include "../../ECS/ECSManager.h"
#include "../../ECS/InternalRegistry/ComponentRegistry.h"

struct CameraComponent : public EComponentS<CameraComponent>{
public:
    static constexpr unsigned int ComponentId = 0;
    static constexpr const char* ComponentGroup = "Rendering";

    bool renderUI;
    int display;

    CameraComponent() = default;
    CameraComponent(int display, bool renderUI) : display(display), renderUI(renderUI) {};
    CameraComponent(const CameraComponent& camera){
        display = camera.display;
        renderUI = camera.renderUI;
    };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), display, true),
        EX_SERIALIZER((*this), renderUI, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"display", display},
            {"renderUI", renderUI}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("display")) display = json["display"].get<int>();
        if (json.contains("renderUI")) renderUI = json["renderUI"].get<bool>();
    }
};

REGISTER_COMPONENT(CameraComponent)