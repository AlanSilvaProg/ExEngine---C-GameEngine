#include "Code/Engine/Core/Engine.h"
#include "Code/Engine/Core/Runtime/App.h"
#include <new>
#include <memory>
#include <string>

#ifdef EXENGINE_EDITOR
#include "Code/Editor/Main/EditorInterface.h"
#endif

int main(int argc, char** argv){
    App *app = new App();
    std::string gamePath;

#ifdef GAME_PATH
    gamePath = GAME_PATH;
#endif

    if(gamePath.empty())
    {
        //gamePath = engineProjectSelector.Initialize();
    }

    // uncomment when project selector be ready!
    // if(gamePath.empty())
    // {
    //     delete(app);
    //     return 0;
    // }

    std::shared_ptr<Engine> engine = std::make_shared<Engine>(gamePath);

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