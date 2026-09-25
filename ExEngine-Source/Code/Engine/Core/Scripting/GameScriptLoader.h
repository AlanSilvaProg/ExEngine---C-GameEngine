#pragma once
#include "DynamicLibrary.h"
#include "../ECS/ECSManager.h"
#include <filesystem>
#include <memory>
#include <vector>

// Loads every compiled script module (.dylib/.so/.dll) found in a shipped Standalone build's
// Scripts folder, exactly once at startup (a Web build instead links scripts straight into this
// binary, so there is nothing to dlopen there - see PlayerBuilder::BuildWeb). Unlike
// ScriptHotReloadManager (Editor-only), there is no file watching, no recompilation and no
// unloading here - a shipped build never edits its own scripts, so the modules just need to stay
// resident for the rest of the process lifetime.
//
// REGISTER_COMPONENT/REGISTER_SYSTEM (ComponentRegistry.h/SystemRegistry.h) both run as a side
// effect of loading a module (Standalone: DynamicLibrary::Load(); Web: the module is already linked
// in, so this already happened before main() ran) - but only *registers a factory* either way.
// Components get instantiated later, per-entity, while a world's data loads; a System has no such
// per-entity trigger, so LoadAll() is also the one place a shipped build ever calls
// SystemRegistry's factories to actually create and attach each script System to the live
// ECSManager (mirrors what ScriptHotReloadManager::ApplyOutcome does after each individual dlopen
// in the Editor).
class GameScriptLoader{
private:
    static std::vector<DynamicLibrary> loadedModules;
public:
    static void LoadAll(const std::filesystem::path& scriptsDirectory, std::shared_ptr<ECSManager> ecsManager);
};
