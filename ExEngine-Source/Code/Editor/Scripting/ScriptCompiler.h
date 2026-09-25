#pragma once
#include <filesystem>
#include <string>

struct ScriptCompileResult{
    bool success = false;
    std::string compilerOutput;
    std::filesystem::path outputLibraryPath;
};

// Compiles a single user script (.hpp defining a Component or a System) into a standalone shared
// library, linked against the same Engine build the Editor itself was compiled against. The flags
// needed for this are baked in at CMake configure time (see Code/Editor/CMakeLists.txt) so this
// class never has to guess the toolchain/include paths used to build the Editor.
class ScriptCompiler{
public:
    // outputDirectory is expected to already exist (typically <project>/Library/ScriptModules).
    // moduleName must be unique per compile (the caller is expected to embed a revision number in
    // it): the previous module for the same script may still be dlopen'd/mapped in memory when a
    // recompile starts, and overwriting that same file on disk while it's mapped is unsafe.
    //
    // linkLibOverride/definesOverride ('|'-separated, matching SCRIPT_DEFINES' own format) default
    // to the Editor's own baked SCRIPT_ENGINE_LINK_LIB/SCRIPT_DEFINES (EXENGINE_EDITOR's Engine
    // build) when left empty. PlayerBuilder passes both explicitly instead, to link an exported
    // project's scripts against the non-editor export template's Engine build.
    static ScriptCompileResult Compile(const std::filesystem::path& scriptPath, const std::filesystem::path& outputDirectory, const std::string& moduleName, const std::string& linkLibOverride = "", const std::string& definesOverride = "");

    // Writes a clangd-compatible compile_flags.txt at the project's root, using the exact same
    // include dirs/defines Compile() uses, so an editor (VSCode/Antigravity/etc.) opened on a lone
    // script under that project's Assets folder resolves engine headers - clangd searches upward
    // from the edited file for this exact filename, so it's picked up regardless of nesting depth.
    static void WriteIntelliSenseConfig(const std::filesystem::path& projectRootPath);
};
