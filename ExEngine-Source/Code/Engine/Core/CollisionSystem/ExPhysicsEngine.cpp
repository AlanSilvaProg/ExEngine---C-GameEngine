#include "ExPhysicsEngine.h"

std::shared_ptr<ECSManager> ExPhysicsEngine::ecsManager;
std::shared_ptr<ClickSystem> ExPhysicsEngine::clickSystem;

void ExPhysicsEngine::Initialize(const std::shared_ptr<ECSManager> ecsManagerInstance){
    ecsManager = ecsManagerInstance;
    if(ecsManager == nullptr) return;

    clickSystem = ecsManager->CreateSystem<ClickSystem>(ecsManagerInstance);
    PhysicsEngineEventHandler::Create();
};

void ExPhysicsEngine::RunCollisionRoutine(){
    if(clickSystem != nullptr) clickSystem->UpdateSystem(SystemContext::EARLY_UPDATE);
};

void ExPhysicsEngine::EntityWasClicked(const unsigned int entityId){
    PhysicsEngineEventHandler::NotifyEntityWasClicked(entityId);
};

void ExPhysicsEngine::Stop(){
    if(clickSystem == nullptr || ecsManager == nullptr) return;
    ecsManager->DestroySystem(typeid(ClickSystem));
    clickSystem = nullptr;
};