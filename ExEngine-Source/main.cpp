#include "Code/Engine/Core/Engine.h"
#include "Code/Engine/Core/EngineGetters.h"
#include "Code/Engine/Core/Runtime/App.h"
#include <new>
#include <memory>
#include <string>

#ifdef EXENGINE_EDITOR
#include "Code/Editor/Main/EditorInterface.h"
#include "Code/Editor/ProjectSelector/ProjectSelector.h"
#endif

int main(int argc, char** argv){
    App *app = new App();
    std::string gamePath;

#ifdef GAME_PATH
    gamePath = GAME_PATH;
#endif

#ifdef EXENGINE_EDITOR
    if(gamePath.empty())
    {
        auto engineProjectSelector = new ProjectSelector();
        gamePath = engineProjectSelector->Run();

        delete(engineProjectSelector);
    }    
#endif

    if(gamePath.empty())
    {
        delete(app);
        return 0;
    }

    std::shared_ptr<Engine> engine = std::make_shared<Engine>();
    EngineGetters engineGetters(engine);

    engine->InitializeEngine();

#ifdef EXENGINE_EDITOR
    EditorInterface *editor = new EditorInterface(engine , gamePath);
#endif

    engine->RunLoop();

    delete(app);

#ifdef EXENGINE_EDITOR
    delete(editor);
#endif

    return 0;
};