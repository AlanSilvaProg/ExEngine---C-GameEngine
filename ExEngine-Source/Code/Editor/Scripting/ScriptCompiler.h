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
    // outputDirectory must already exist. moduleName must be unique per compile (embed a revision
    // number) - the previous module may still be mapped in memory when overwriting starts.
    // linkLibOverride/definesOverride default to the Editor's own Engine build when empty;
    // PlayerBuilder passes both explicitly to target an export template's Engine build instead.
    static ScriptCompileResult Compile(const std::filesystem::path& scriptPath, const std::filesystem::path& outputDirectory, const std::string& moduleName, const std::string& linkLibOverride = "", const std::string& definesOverride = "");

    // Writes a clangd-compatible compile_flags.txt at the project's root, using the exact same
    // include dirs/defines Compile() uses, so an editor (VSCode/Antigravity/etc.) opened on a lone
    // script under that project's Assets folder resolves engine headers - clangd searches upward
    // from the edited file for this exact filename, so it's picked up regardless of nesting depth.
    static void WriteIntelliSenseConfig(const std::filesystem::path& projectRootPath);
};
