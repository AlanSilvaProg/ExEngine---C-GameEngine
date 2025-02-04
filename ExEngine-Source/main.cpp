#include "Code/Engine/Core/Engine.h"
#include "Code/Editor/Main/EditorInterface.h"
#include <new>

int main(){
    Engine *engine = new Engine();
    engine->InitializeEngine();

    EditorInterface *editor = new EditorInterface();
    engine->RunLoop();

    delete(engine);
    delete(editor);
    return 0;
};