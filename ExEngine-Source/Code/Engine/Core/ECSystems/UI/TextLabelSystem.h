#pragma once
#include "../../ECS/ECSManager.h"

class TextLabelSystem : public ECSystem{
public:
    TextLabelSystem();

    void UpdateSystem(SystemContext systemContext) override;
    
    inline const char* SystemName() override { return TYPE_NAME(TextLabelSystem); }; 
};