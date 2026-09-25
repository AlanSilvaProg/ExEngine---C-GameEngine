#include "DynamicLibrary.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

DynamicLibrary::~DynamicLibrary(){
    Unload();
};

DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept : handle(other.handle){
    other.handle = nullptr;
};

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept{
    if(this == &other) return *this;

    Unload();
    handle = other.handle;
    other.handle = nullptr;

    return *this;
};

bool DynamicLibrary::Load(const std::filesystem::path& libraryPath, std::string& outError){
    Unload();

#ifdef _WIN32
    handle = static_cast<void*>(LoadLibraryW(libraryPath.wstring().c_str()));
    if(handle == nullptr)
    {
        outError = "LoadLibrary failed with error code: " + std::to_string(GetLastError());
        return false;
    }
#else
    // RTLD_NOW resolves every symbol immediately, so a real link error (missing Engine symbol,
    // ABI mismatch) surfaces right here instead of crashing later when the symbol is first touched.
    handle = dlopen(libraryPath.c_str(), RTLD_NOW | RTLD_LOCAL);
    if(handle == nullptr)
    {
        const char* error = dlerror();
        outError = error != nullptr ? error : "dlopen failed with an unknown error";
        return false;
    }
#endif

    return true;
};

void DynamicLibrary::Unload(){
    if(handle == nullptr) return;

#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif

    handle = nullptr;
};
