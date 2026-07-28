#pragma once
#include "../../../Engine/Core/ECS/ECSManager.h"

struct EditorCameraComponent : public EComponentS<EditorCameraComponent>{
public:
    static constexpr unsigned int ComponentId = 3;

    virtual ExSerializedClass Serialize() override{
        return ExSerializedClass{
            Demangle(typeid(*this).name()),
            {
            }
        };
    };
    
    virtual nlohmann::json ToJson() override {
        return {};
    }

    virtual void FromJson(const nlohmann::json& json) override {
    }
};