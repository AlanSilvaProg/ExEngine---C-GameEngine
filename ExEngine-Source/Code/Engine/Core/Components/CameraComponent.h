#pragma once
#include "../ECS/ECSManager.h"
#include "../ECS/InternalRegistry/ComponentRegistry.h"

struct CameraComponent : public EComponentS<CameraComponent>{
public:
    int display;

    CameraComponent() = default;
    CameraComponent(int display) : display(display) {};

    virtual ExSerializedClass Serialize() override{
        return ExSerializedClass{
            Demangle(typeid(*this).name()),
            {
                EX_SERIALIZER((*this), display, true)
            }
        };
    };
};

REGISTER_COMPONENT(CameraComponent)