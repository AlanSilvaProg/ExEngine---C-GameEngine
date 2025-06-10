#include <new>
#include <memory>
#include "Code/Engine/Core/Engine.h"
#include "Code/Engine/Core/Runtime/App.h"
#include "Code/Editor/Main/EditorInterface.h"

int main(){
    App *app = new App();
    std::shared_ptr<Engine> engine = std::make_shared<Engine>();
    engine->InitializeEngine();

#ifdef EXENGINE_EDITOR
    EditorInterface *editor = new EditorInterface(engine);
#endif

    engine->RunLoop();

    delete(app);

#ifdef EXENGINE_EDITOR
    delete(editor);
#endif

    return 0;
};