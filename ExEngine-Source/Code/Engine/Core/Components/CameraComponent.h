#pragma once
#include "../ECS/ECSManager.h"
#include "../ECS/InternalRegistry/ComponentRegistry.h"

struct CameraComponent : public EComponentS<CameraComponent>{
public:
    static constexpr unsigned int ComponentId = 0;

    int display;

    CameraComponent() = default;
    CameraComponent(int display) : display(display) {};
    CameraComponent(const CameraComponent& camera){
        display = camera.display;
    };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), display, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"display", display}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("display")) display = json["display"].get<int>();
    }
};

REGISTER_COMPONENT(CameraComponent)