#pragma once
#include "../Rendering/Systems/RenderingSystem2D.h"
#include "../ECS/ECSManager.h"
#include "glm/glm.hpp"

class ClickSystem : public ECSystem{
private:
    std::shared_ptr<ECSManager> ecsManager;
public:
    ClickSystem(std::shared_ptr<ECSManager> ecsManager);

    void UpdateSystem(SystemContext systemContext) override;
    bool CheckBounds(const std::shared_ptr<EntityCS> entity, const glm::vec2& worldMousePosition) const;
    inline const char* SystemName() override { return TYPE_NAME(ClickSystem); }; 
};