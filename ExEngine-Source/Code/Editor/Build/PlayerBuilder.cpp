#include "PlayerBuilder.h"
#include "../Scripting/ScriptCompiler.h"
#include "../Scripting/ProcessRunner.h"
#include "../../Engine/File/FileManagement.h"
#include "../../Engine/Logger/Logger.h"
#include <fstream>
#include <cstdlib>

#ifndef ENGINE_SOURCE_ROOT
#define ENGINE_SOURCE_ROOT ""
#endif

namespace{
    // GLM has no CMake config package Emscripten's find_package can see (see root CMakeLists.txt),
    // so the whole project - including this - just points straight at the Homebrew keg.
    const char* GLM_INCLUDE_DIR = "/opt/homebrew/Cellar/glm/1.0.0/include";

    inline void Report(const BuildStepCallback& onStep, const std::string& description, int stepIndex, int totalSteps){
        if(onStep) onStep(description, stepIndex, totalSteps);
    }
}

std::string PlayerBuilder::PlatformScriptDefines(bool devMode){
    std::string defines =
#ifdef __APPLE__
        "EXENGINE_MACOS";
#elif defined(_WIN32)
        "EXENGINE_WINDOWS";
#else
        "EXENGINE_LINUX";
#endif

    if(devMode) defines += "|EXENGINE_DEBUG_MODE";
    return defines;
};

int PlayerBuilder::CountProjectScripts(const std::filesystem::path& projectPath){
    auto scriptsPath = projectPath / "Assets" / "Scripts";
    if(!std::filesystem::exists(scriptsPath)) return 0;

    int count = 0;
    for(const auto& entry : std::filesystem::recursive_directory_iterator(scriptsPath))
    {
        if(!entry.is_directory() && entry.path().extension() == ".hpp") count++;
    }
    return count;
};

std::filesystem::path PlayerBuilder::GetTemplateBuildDir(bool isWeb, bool devMode){
    std::filesystem::path root = ENGINE_SOURCE_ROOT;
    return root / "build" / "_ExportTemplates" / (isWeb ? "Web" : "Standalone") / (devMode ? "Dev" : "Release");
};

void PlayerBuilder::CleanTemplateCache(bool isWeb, bool devMode){
    auto templateBuildDir = GetTemplateBuildDir(isWeb, devMode);
    if(!std::filesystem::exists(templateBuildDir)) return;

    Logger::Log("PlayerBuilder: cleaning cached " + std::string(isWeb ? "Web " : "Standalone ") + (devMode ? "Dev" : "Release") + " export template at " + templateBuildDir.string());
    std::filesystem::remove_all(templateBuildDir);
};

void PlayerBuilder::WriteExportedProjectManifest(const ProjectInfo& projectInfo, const std::filesystem::path& outputDir){
    // Only what main.cpp's non-editor path actually reads (defaultWorld) - not the full editor
    // ProjectInfo (projectPath there points at the dev machine's project folder, meaningless here).
    ProjectInfo exportedInfo;
    exportedInfo.projectName = projectInfo.projectName;
    exportedInfo.defaultWorld = projectInfo.defaultWorld;
    FileManagement::SaveFile(outputDir / "ExProject.exproj", exportedInfo.ToJson().dump());
};

bool PlayerBuilder::EnsureStandaloneTemplate(bool devMode, std::filesystem::path& outEngineLibraryPath, std::filesystem::path& outExecutablePath, const BuildStepCallback& onStep, int stepIndex, int totalSteps, std::string& outError){
    Report(onStep, devMode ? "Ensuring Standalone Dev export template is built..." : "Ensuring Standalone Release export template is built...", stepIndex, totalSteps);

    std::filesystem::path root = ENGINE_SOURCE_ROOT;
    std::filesystem::path templateBuildDir = GetTemplateBuildDir(false, devMode);

    // Root CMakeLists.txt turns this into a .app bundle on macOS when DEBUG_MODE is off (Release
    // export) - only the raw binary is taken out below, so the bundle wrapper is otherwise ignored.
#ifdef _WIN32
    outEngineLibraryPath = templateBuildDir / "Code/Engine/Engine.dll";
    outExecutablePath = templateBuildDir / "ExGameEngineWin.exe";
#elif defined(__APPLE__)
    outEngineLibraryPath = templateBuildDir / "Code/Engine/libEngine.dylib";
    outExecutablePath = devMode
        ? templateBuildDir / "ExGameEngineMac"
        : templateBuildDir / "ExGameEngineMac.app/Contents/MacOS/ExGameEngineMac";
#else
    outEngineLibraryPath = templateBuildDir / "Code/Engine/libEngine.so";
    outExecutablePath = templateBuildDir / "ExGameEngineLinux";
#endif

    if(std::filesystem::exists(outEngineLibraryPath) && std::filesystem::exists(outExecutablePath))
        return true;

    Logger::Log("PlayerBuilder: building the Standalone " + std::string(devMode ? "Dev" : "Release") + " export template (first time only - every export from now on reuses it)...");

    auto configureResult = ProcessRunner::Run({
        "cmake", "-S", root.string(), "-B", templateBuildDir.string(),
        "-DEDITOR=OFF",
        std::string("-DDEBUG_MODE=") + (devMode ? "ON" : "OFF"),
        std::string("-DCMAKE_BUILD_TYPE=") + (devMode ? "Debug" : "Release")
    });
    if(!configureResult.Succeeded())
    {
        outError = "Standalone template configure failed:\n" + configureResult.output;
        return false;
    }

    auto buildResult = ProcessRunner::Run({"cmake", "--build", templateBuildDir.string()});
    if(!buildResult.Succeeded())
    {
        outError = "Standalone template build failed:\n" + buildResult.output;
        return false;
    }

    if(!std::filesystem::exists(outEngineLibraryPath) || !std::filesystem::exists(outExecutablePath))
    {
        outError = "Standalone template build finished but the expected output is missing (looked for "
            + outEngineLibraryPath.string() + " and " + outExecutablePath.string() + ")";
        return false;
    }

    return true;
};

bool PlayerBuilder::CompileProjectScriptsStandalone(const std::filesystem::path& projectPath, const std::filesystem::path& engineLibraryPath, const std::filesystem::path& outputScriptsDir, bool devMode, const BuildStepCallback& onStep, int firstStepIndex, int totalSteps, std::string& outError){
    auto scriptsPath = projectPath / "Assets" / "Scripts";
    if(!std::filesystem::exists(scriptsPath)) return true;

    auto defines = PlatformScriptDefines(devMode);
    int stepIndex = firstStepIndex;

    for(const auto& entry : std::filesystem::recursive_directory_iterator(scriptsPath))
    {
        if(entry.is_directory() || entry.path().extension() != ".hpp") continue;

        Report(onStep, "Compiling script: " + entry.path().filename().string(), stepIndex++, totalSteps);

        auto moduleName = entry.path().stem().string();
        auto compileResult = ScriptCompiler::Compile(entry.path(), outputScriptsDir, moduleName, engineLibraryPath.string(), defines);

        if(!compileResult.success)
        {
            outError = "Failed to compile script '" + entry.path().filename().string() + "':\n" + compileResult.compilerOutput;
            return false;
        }
    }

    return true;
};

bool PlayerBuilder::BuildStandalone(const std::filesystem::path& projectPath, const ProjectInfo& projectInfo, const std::filesystem::path& outputDir, bool devMode, const BuildStepCallback& onStep, std::string& outError){
    const int scriptCount = CountProjectScripts(projectPath);
    const int totalSteps = 3 + scriptCount; // template, copy player+assets, N scripts, finalize
    int step = 0;

    std::filesystem::path engineLibraryPath, templateExecutablePath;
    if(!EnsureStandaloneTemplate(devMode, engineLibraryPath, templateExecutablePath, onStep, ++step, totalSteps, outError))
        return false;

    Report(onStep, "Copying player and assets...", ++step, totalSteps);

    std::filesystem::create_directories(outputDir);

    // The reusable player template - identical for every project, nothing here is project-specific.
    std::filesystem::copy(templateExecutablePath, outputDir / templateExecutablePath.filename(), std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy(engineLibraryPath, outputDir / engineLibraryPath.filename(), std::filesystem::copy_options::overwrite_existing);

#ifndef _WIN32
    std::filesystem::permissions(outputDir / templateExecutablePath.filename(),
        std::filesystem::perms::owner_all | std::filesystem::perms::group_read | std::filesystem::perms::group_exec | std::filesystem::perms::others_read | std::filesystem::perms::others_exec,
        std::filesystem::perm_options::add);
#endif

    auto projectAssetsPath = projectPath / "Assets";
    if(std::filesystem::exists(projectAssetsPath))
        std::filesystem::copy(projectAssetsPath, outputDir / "Assets", std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    // Engine-owned assets (e.g. the default font) ship with every game build regardless of
    // project, so they're merged into the same Assets folder alongside the project's own.
    auto engineAssetsPath = std::filesystem::path(ENGINE_SOURCE_ROOT) / "Assets" / "Engine";
    if(std::filesystem::exists(engineAssetsPath))
        std::filesystem::copy(engineAssetsPath, outputDir / "Assets" / "Engine", std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    auto scriptsOutputDir = outputDir / "Scripts";
    std::filesystem::create_directories(scriptsOutputDir);
    if(!CompileProjectScriptsStandalone(projectPath, engineLibraryPath, scriptsOutputDir, devMode, onStep, step + 1, totalSteps, outError))
        return false;
    step += scriptCount;

    Report(onStep, "Finalizing build...", ++step, totalSteps);
    WriteExportedProjectManifest(projectInfo, outputDir);

    Logger::Log("PlayerBuilder: Standalone build finished at " + outputDir.string());
    return true;
};

bool PlayerBuilder::EnsureWebTemplate(bool devMode, std::filesystem::path& outEngineLibraryPath, std::filesystem::path& outMainObjectPath, const BuildStepCallback& onStep, int stepIndex, int totalSteps, std::string& outError){
    Report(onStep, devMode ? "Ensuring Web Dev export template is built..." : "Ensuring Web Release export template is built...", stepIndex, totalSteps);

    std::filesystem::path root = ENGINE_SOURCE_ROOT;
    std::filesystem::path templateBuildDir = GetTemplateBuildDir(true, devMode);

    outEngineLibraryPath = templateBuildDir / "Code/Engine/libEngine.a";
    outMainObjectPath = templateBuildDir / "CMakeFiles/ExEngine-Source.dir/main.cpp.o";

    if(std::filesystem::exists(outEngineLibraryPath) && std::filesystem::exists(outMainObjectPath))
        return true;

    Logger::Log("PlayerBuilder: building the Web " + std::string(devMode ? "Dev" : "Release") + " export template (first time only - every export from now on reuses it)...");

#ifdef __APPLE__
    // emcmake/emcc otherwise pick up whatever "python3" is first on PATH, which on this machine is
    // an old system Python the Emscripten toolchain can't run under (see CMakeLists.txt history).
    setenv("EMSDK_PYTHON", "/opt/homebrew/bin/python3", 1);
#endif

    auto configureResult = ProcessRunner::Run({
        "emcmake", "cmake", "-S", root.string(), "-B", templateBuildDir.string(),
        "-DEDITOR=OFF",
        std::string("-DDEBUG_MODE=") + (devMode ? "ON" : "OFF"),
        std::string("-DCMAKE_BUILD_TYPE=") + (devMode ? "Debug" : "Release")
    });
    if(!configureResult.Succeeded())
    {
        outError = "Web template configure failed:\n" + configureResult.output;
        return false;
    }

    auto buildResult = ProcessRunner::Run({"cmake", "--build", templateBuildDir.string()});
    if(!buildResult.Succeeded())
    {
        outError = "Web template build failed:\n" + buildResult.output;
        return false;
    }

    if(!std::filesystem::exists(outEngineLibraryPath) || !std::filesystem::exists(outMainObjectPath))
    {
        outError = "Web template build finished but the expected output is missing (looked for "
            + outEngineLibraryPath.string() + " and " + outMainObjectPath.string() + ")";
        return false;
    }

    return true;
};

bool PlayerBuilder::BuildWeb(const std::filesystem::path& projectPath, const ProjectInfo& projectInfo, const std::filesystem::path& outputDir, bool devMode, const BuildStepCallback& onStep, std::string& outError){
    const int scriptCount = CountProjectScripts(projectPath);
    const int totalSteps = 3 + scriptCount; // template, N scripts, link, finalize
    int step = 0;

    std::filesystem::path engineLibraryPath, mainObjectPath;
    if(!EnsureWebTemplate(devMode, engineLibraryPath, mainObjectPath, onStep, ++step, totalSteps, outError))
        return false;

    std::filesystem::create_directories(outputDir);

#ifdef __APPLE__
    setenv("EMSDK_PYTHON", "/opt/homebrew/bin/python3", 1);
#endif

    // A browser can't dlopen new code into an already-running wasm module, so - unlike Standalone -
    // scripts compile straight to object files and link directly into the final binary below,
    // alongside the template's prebuilt (never recompiled) static Engine library.
    auto generatedDir = outputDir / "_generated";
    std::filesystem::create_directories(generatedDir);

    auto jsonIncludeDir = GetTemplateBuildDir(true, devMode) / "_deps/json-src/include";

    std::vector<std::string> scriptObjectPaths;
    auto scriptsPath = projectPath / "Assets" / "Scripts";

    if(std::filesystem::exists(scriptsPath))
    {
        for(const auto& entry : std::filesystem::recursive_directory_iterator(scriptsPath))
        {
            if(entry.is_directory() || entry.path().extension() != ".hpp") continue;

            Report(onStep, "Compiling script: " + entry.path().filename().string(), ++step, totalSteps);

            auto trampolinePath = generatedDir / (entry.path().stem().string() + ".generated.cpp");
            auto objectPath = generatedDir / (entry.path().stem().string() + ".o");

            std::ofstream trampolineFile(trampolinePath);
            if(!trampolineFile.is_open())
            {
                outError = "Failed to write trampoline for script: " + entry.path().filename().string();
                return false;
            }
            trampolineFile << "#include \"" << entry.path().generic_string() << "\"\n";
            trampolineFile.close();

            std::vector<std::string> compileArgv = {
                "em++", "-std=c++20", "-sUSE_SDL=2", "-sUSE_SDL_TTF=2", "-sUSE_SDL_IMAGE=2", "-sSDL2_IMAGE_FORMATS=png,jpg", "-pthread",
                "-I", ENGINE_SOURCE_ROOT,
                "-I", (std::filesystem::path(ENGINE_SOURCE_ROOT) / "Code/Engine").string(),
                "-I", jsonIncludeDir.string(),
                "-I", GLM_INCLUDE_DIR
            };
            if(devMode) compileArgv.push_back("-DEXENGINE_DEBUG_MODE");
            compileArgv.push_back("-c");
            compileArgv.push_back(trampolinePath.string());
            compileArgv.push_back("-o");
            compileArgv.push_back(objectPath.string());

            auto compileResult = ProcessRunner::Run(compileArgv);

            if(!compileResult.Succeeded())
            {
                outError = "Failed to compile script '" + entry.path().filename().string() + "':\n" + compileResult.output;
                return false;
            }

            scriptObjectPaths.push_back(objectPath.string());
        }
    }

    Report(onStep, "Linking final build...", ++step, totalSteps);

    auto outputName = projectInfo.projectName.empty() ? std::string("Game") : projectInfo.projectName;
    auto outputHtmlPath = outputDir / (outputName + ".html");

    // Replaces Emscripten's default shell (spinner/status/progress bar/resize controls) with a
    // minimal one that's just a fullscreen canvas - see Resources/Web/shell.html.
    auto shellFilePath = std::filesystem::path(ENGINE_SOURCE_ROOT) / "Resources" / "Web" / "shell.html";

    std::vector<std::string> linkArgv = {
        "em++", "-sUSE_SDL=2", "-sUSE_SDL_TTF=2", "-sUSE_SDL_IMAGE=2", "-sSDL2_IMAGE_FORMATS=png,jpg", "-sFETCH=1", "-pthread",
        "-sPTHREAD_POOL_SIZE=4", "-lwebsocket.js",
        "--shell-file", shellFilePath.string(),
        mainObjectPath.string()
    };
    // Keeps real function names in the wasm's Name section (cheap - no full DWARF/-g) so a crash's
    // browser console stack trace reads as actual call names instead of bare "NewProject.wasm:0x..."
    // offsets nobody can act on. Dev-only: no reason to pay even this for a Release export.
    if(devMode) linkArgv.push_back("--profiling-funcs");
    for(const auto& objectPath : scriptObjectPaths) linkArgv.push_back(objectPath);
    linkArgv.push_back(engineLibraryPath.string());

    auto projectAssetsPath = projectPath / "Assets";
    if(std::filesystem::exists(projectAssetsPath))
    {
        linkArgv.push_back("--preload-file");
        linkArgv.push_back(projectAssetsPath.string() + "@Assets");
    }

    // Engine-owned assets (e.g. the default font) ship with every game build regardless of
    // project, so they're preloaded into the same Assets folder alongside the project's own.
    auto engineAssetsPath = std::filesystem::path(ENGINE_SOURCE_ROOT) / "Assets" / "Engine";
    if(std::filesystem::exists(engineAssetsPath))
    {
        linkArgv.push_back("--preload-file");
        linkArgv.push_back(engineAssetsPath.string() + "@Assets/Engine");
    }

    // main.cpp reads this at Engine::GetEnginePath()/"ExProject.exproj", which on Emscripten is
    // the MEMFS root - must exist before main() runs, so it's preloaded here rather than only
    // written to outputDir after linking (or no manifest loads, meaning no camera/black canvas).
    auto manifestPath = generatedDir / "ExProject.exproj";
    WriteExportedProjectManifest(projectInfo, generatedDir);
    linkArgv.push_back("--preload-file");
    linkArgv.push_back(manifestPath.string() + "@ExProject.exproj");

    linkArgv.push_back("-o");
    linkArgv.push_back(outputHtmlPath.string());

    auto linkResult = ProcessRunner::Run(linkArgv);
    if(!linkResult.Succeeded())
    {
        outError = "Web link failed:\n" + linkResult.output;
        return false;
    }

    Report(onStep, "Finalizing build...", ++step, totalSteps);
    WriteExportedProjectManifest(projectInfo, outputDir);

    Logger::Log("PlayerBuilder: Web build finished at " + outputHtmlPath.string());
    return true;
};
