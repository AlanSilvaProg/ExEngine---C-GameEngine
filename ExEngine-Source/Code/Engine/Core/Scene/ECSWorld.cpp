#include "ECSWorld.h"

bool ECSWorld::SaveCurrentState(){
    //ToDo save world
    return false;
};

ECSWorldInfo& ECSWorld::LoadState(){
    ResetWorld();
    //ToDo load world
    GenerateWorldEntities();
    return worldInformation;
}; 

void ECSWorld::GenerateWorldEntities(){
    //ToDo generate world
};

void ECSWorld::ResetWorld(){
    //ToDo reset world
};