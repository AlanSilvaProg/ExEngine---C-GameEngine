#include "Code/Engine/Core/Engine.h"
#include "Code/Engine/Core/Runtime/App.h"
#include "Code/Editor/Main/EditorInterface.h"
#include <new>

int main(){
    App *app = new App();
    Engine *engine = new Engine();
    engine->InitializeEngine();

#ifdef ExEngineEditor
    EditorInterface *editor = new EditorInterface();
#endif

    engine->RunLoop();

    delete(app);
    delete(engine);

#ifdef ExEngineEditor
    delete(editor);
#endif

    return 0;
};