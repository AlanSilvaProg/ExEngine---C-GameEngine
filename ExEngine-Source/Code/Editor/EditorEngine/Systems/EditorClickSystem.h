#pragma once
#include "../../../Engine/Core/ECS/ECSManager.h"
#include "../../../Engine/Core/Components/BoxColliderComponent.h"
#include "../../../Engine/Core/Components/TransformComponent.h"
#include "../../Main/Windows/EditorWindows/EntityBrowser/EntityBrowserSelection.h"
#include <glm/glm.hpp>
#include <unordered_set>
#include <vector>
#include <memory>

class EditorClickSystem : public ECSystem{
private:
    EntityBrowserSelection entitySelection;
    std::unordered_set<unsigned int> ignoredEntityIds;
    double mouseDownTime = 0.0;

    bool TrySelectEntity(const std::vector<std::shared_ptr<EntityCS>>& entities, const glm::vec2& worldMousePosition);
public:
    EditorClickSystem();

    void UpdateSystem(SystemContext systemContext) override;
    bool CheckBounds(const std::shared_ptr<EntityCS> entity, const glm::vec2& worldMousePosition) const;
    inline const char* SystemName() override { return TYPE_NAME(EditorClickSystem); };
};
