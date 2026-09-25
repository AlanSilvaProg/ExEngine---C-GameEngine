#include "Code/Engine/Core/Engine.h"
#include "Code/Engine/Core/EngineGetters.h"
#include "Code/Engine/Core/Runtime/App.h"
#include "Code/Engine/Logger/StackTrace/StackTrace.h"
#include <new>
#include <memory>
#include <string>
#include <filesystem>

#ifdef EXENGINE_EDITOR
#include "Code/Editor/Main/EditorInterface.h"
#include "Code/Editor/ProjectSelector/ProjectSelector.h"
#else
#include "Code/Engine/Core/Project/ProjectInfo.h"
#include "Code/Engine/Core/Scene/ECSWorldManager.h"
#include "Code/Engine/Core/Scripting/GameScriptLoader.h"
#include "Code/Engine/File/FileManagement.h"
#endif

int main(int argc, char** argv){
    StackTrace::InstallCrashHandler();

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
#else
    // A shipped (non-editor) build ships as a reusable template exported next to a project's own
    // Assets/Scripts (see Code/Editor/Build/PlayerBuilder.*) rather than being recompiled per
    // project, so GAME_PATH usually isn't baked in - fall back to "wherever this executable is".
    if(gamePath.empty())
        gamePath = Engine::GetEnginePath().string();
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
#else
    GameScriptLoader::LoadAll(std::filesystem::path(gamePath) / "Scripts", engine->GetECSManagerPtr());

    ProjectInfo projectInfo;
    if(FileManagement::LoadFromJson(std::filesystem::path(gamePath) / "ExProject.exproj", projectInfo) && !projectInfo.defaultWorld.empty())
    {
        auto worldPath = std::filesystem::path(gamePath) / "Assets/Worlds" / projectInfo.defaultWorld;
        worldPath.replace_extension(".exworld");
        ECSWorldManager::LoadWorld(worldPath);
    }
#endif

    StackTrace::InstallCrashHandler();

    engine->RunLoop();
    
    StackTrace::UninstallCrashHandler();

    delete(app);

#ifdef EXENGINE_EDITOR
    delete(editor);
#endif

    return 0;
};