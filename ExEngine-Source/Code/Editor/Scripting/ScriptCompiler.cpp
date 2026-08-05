#include "ScriptCompiler.h"
#include "ProcessRunner.h"
#include <fstream>
#include <sstream>
#include <vector>

#ifndef SCRIPT_CXX_COMPILER
#define SCRIPT_CXX_COMPILER ""
#endif
#ifndef SCRIPT_CXX_COMPILER_ID
#define SCRIPT_CXX_COMPILER_ID ""
#endif
#ifndef SCRIPT_CXX_STANDARD
#define SCRIPT_CXX_STANDARD "20"
#endif
#ifndef SCRIPT_ENGINE_LINK_LIB
#define SCRIPT_ENGINE_LINK_LIB ""
#endif
#ifndef SCRIPT_INCLUDE_DIRS
#define SCRIPT_INCLUDE_DIRS ""
#endif
#ifndef SCRIPT_DEFINES
#define SCRIPT_DEFINES ""
#endif

namespace{
    std::vector<std::string> SplitPipeList(const std::string& value){
        std::vector<std::string> result;
        std::stringstream stream(value);
        std::string item;
        while(std::getline(stream, item, '|'))
        {
            if(!item.empty()) result.push_back(item);
        }
        return result;
    };

    std::string PlatformLibraryExtension(){
#ifdef _WIN32
        return ".dll";
#elif defined(__APPLE__)
        return ".dylib";
#else
        return ".so";
#endif
    };
}

ScriptCompileResult ScriptCompiler::Compile(const std::filesystem::path& scriptPath, const std::filesystem::path& outputDirectory, const std::string& moduleName){
    ScriptCompileResult result;

    auto trampolinePath = outputDirectory / (moduleName + ".generated.cpp");
    auto outputLibraryPath = outputDirectory / (moduleName + PlatformLibraryExtension());

    // The .hpp itself is never compiled directly - a tiny generated .cpp including it is the
    // actual translation unit, which keeps this independent of any compiler-specific flag for
    // treating a header as a source file. Written directly (not via FileManagement::CreateFile,
    // which deliberately never overwrites an existing file) since this cache file must be
    // regenerated fresh on every recompile.
    std::ofstream trampolineFile(trampolinePath);
    if(!trampolineFile.is_open())
    {
        result.compilerOutput = "Failed to write trampoline file at: " + trampolinePath.string();
        return result;
    }
    trampolineFile << "#include \"" << scriptPath.generic_string() << "\"\n";
    trampolineFile.close();

    auto includeDirs = SplitPipeList(SCRIPT_INCLUDE_DIRS);
    auto defines = SplitPipeList(SCRIPT_DEFINES);
    std::string compilerId = SCRIPT_CXX_COMPILER_ID;
    std::string compilerPath = SCRIPT_CXX_COMPILER;
    std::string linkLib = SCRIPT_ENGINE_LINK_LIB;

    std::vector<std::string> argv;
    argv.push_back(compilerPath);

    if(compilerId == "MSVC")
    {
        argv.push_back("/nologo");
        argv.push_back("/LD");
        argv.push_back("/EHsc");
        argv.push_back(std::string("/std:c++") + SCRIPT_CXX_STANDARD);

        for(const auto& dir : includeDirs) argv.push_back("/I" + dir);
        for(const auto& define : defines) argv.push_back("/D" + define);

        argv.push_back(trampolinePath.string());
        argv.push_back(linkLib);
        argv.push_back("/Fe:" + outputLibraryPath.string());
    }
    else
    {
        argv.push_back(std::string("-std=c++") + SCRIPT_CXX_STANDARD);
        argv.push_back("-fPIC");
        argv.push_back("-shared");

        for(const auto& dir : includeDirs) argv.push_back("-I" + dir);
        for(const auto& define : defines) argv.push_back("-D" + define);

        argv.push_back(trampolinePath.string());
        argv.push_back(linkLib);

#ifdef __linux__
        // On Linux the .so dependency is recorded by path/soname; pin an rpath as a safety net so
        // dlopen() on the compiled script can always find libEngine.so even if it isn't already
        // resident in the process for some reason.
        argv.push_back("-Wl,-rpath," + std::filesystem::path(linkLib).parent_path().string());
#endif

        argv.push_back("-o");
        argv.push_back(outputLibraryPath.string());
    }

    auto processResult = ProcessRunner::Run(argv);
    result.compilerOutput = processResult.output;
    result.success = processResult.Succeeded() && std::filesystem::exists(outputLibraryPath);

    if(result.success) result.outputLibraryPath = outputLibraryPath;

    return result;
};
