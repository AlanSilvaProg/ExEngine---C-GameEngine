// Demangle.h
#pragma once
#include <string>
#include <typeinfo>

#if defined(_MSC_VER)

inline std::string Demangle(const char* name)                { return name; }
inline std::string Demangle(const std::type_info& ti)        { return ti.name(); }

#else
// GCC/Clang (libstdc++ / libc++)
#include <cxxabi.h>
#include <cstdlib>


inline std::string Demangle(const char* name) {
    int status = 0;
    size_t len = 0;
    char* buf = abi::__cxa_demangle(name, nullptr, &len, &status);
    std::string out = (status == 0 && buf) ? std::string(buf) : std::string(name);
    std::free(buf);
    return out;
}

inline std::string Demangle(const std::type_info& ti) {
    return Demangle(ti.name());
}
#endif