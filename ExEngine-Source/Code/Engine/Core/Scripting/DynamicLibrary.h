#pragma once
#include <filesystem>
#include <string>

// RAII wrapper around dlopen/dlclose (POSIX) or LoadLibrary/FreeLibrary (Windows).
// There is no symbol lookup here on purpose: REGISTER_COMPONENT/REGISTER_SYSTEM activate through
// global static initializers, which the loader already runs as part of Load() - no dlsym/GetProcAddress
// needed for a script module to make itself known to ComponentRegistry/SystemRegistry.
class DynamicLibrary{
private:
    void* handle = nullptr;
public:
    DynamicLibrary() = default;
    ~DynamicLibrary();

    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    DynamicLibrary(DynamicLibrary&& other) noexcept;
    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;

    bool Load(const std::filesystem::path& libraryPath, std::string& outError);
    void Unload();

    inline bool IsLoaded() const { return handle != nullptr; };
};
