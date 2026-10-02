#pragma once
#include "../../Engine/Core/Project/ProjectInfo.h"
#include <filesystem>
#include <functional>
#include <string>

// Reports progress before each build step: (description, stepIndex (1-based), totalSteps).
using BuildStepCallback = std::function<void(const std::string&, int, int)>;

// Exports a project into a self-contained build (Unity/Godot's "export template" model): the
// engine compiles once per (platform, devMode) and is cached; each export only rebuilds the
// project's own Scripts/Assets. Web can't dlopen, so scripts link into the wasm directly. Runs
// synchronously - PlayerBuildRunner moves this off the main thread.
class PlayerBuilder{
public:
    static bool BuildStandalone(const std::filesystem::path& projectPath, const ProjectInfo& projectInfo, const std::filesystem::path& outputDir, bool devMode, const BuildStepCallback& onStep, std::string& outError);
    static bool BuildWeb(const std::filesystem::path& projectPath, const ProjectInfo& projectInfo, const std::filesystem::path& outputDir, bool devMode, const BuildStepCallback& onStep, std::string& outError);

    // Deletes the cached export template for (isWeb, devMode), forcing the next build for that pair
    // to recompile it from scratch instead of reusing whatever is on disk - e.g. after an engine-side
    // change a stale cached template wouldn't otherwise pick up ("Clean Build" in the Build window).
    static void CleanTemplateCache(bool isWeb, bool devMode);

private:
    static std::filesystem::path GetTemplateBuildDir(bool isWeb, bool devMode);

    static bool EnsureStandaloneTemplate(bool devMode, std::filesystem::path& outEngineLibraryPath, std::filesystem::path& outExecutablePath, const BuildStepCallback& onStep, int stepIndex, int totalSteps, std::string& outError);
    static bool EnsureWebTemplate(bool devMode, std::filesystem::path& outEngineLibraryPath, std::filesystem::path& outMainObjectPath, const BuildStepCallback& onStep, int stepIndex, int totalSteps, std::string& outError);

    static bool CompileProjectScriptsStandalone(const std::filesystem::path& projectPath, const std::filesystem::path& engineLibraryPath, const std::filesystem::path& outputScriptsDir, bool devMode, const BuildStepCallback& onStep, int firstStepIndex, int totalSteps, std::string& outError);
    static std::string PlatformScriptDefines(bool devMode);
    static int CountProjectScripts(const std::filesystem::path& projectPath);

    static void WriteExportedProjectManifest(const ProjectInfo& projectInfo, const std::filesystem::path& outputDir);
};
