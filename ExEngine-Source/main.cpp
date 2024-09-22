#include "Code/Engine/Core/Engine.h"
#include <new>

int main(){
    Engine *engine = new Engine();

    engine->InitializeEngine();

    delete(engine);
    return 0;
};