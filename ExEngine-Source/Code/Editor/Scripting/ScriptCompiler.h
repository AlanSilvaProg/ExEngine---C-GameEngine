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
    static ScriptCompileResult Compile(const std::filesystem::path& scriptPath, const std::filesystem::path& outputDirectory, const std::string& moduleName);
};
