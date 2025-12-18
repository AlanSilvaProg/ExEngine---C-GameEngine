#pragma once
#include <filesystem>

#ifndef BASE_LUA_ECSYSTEM
#define BASE_LUA_ECSYSTEM "\
local variable = 10 \n\
\n\
print(variable.. \"ECSystem\")\n\
\
"
#endif

#ifndef BASE_LUA_ECOMPONENTS
#define BASE_LUA_ECOMPONENTS "\
local variable = 5\n\
\n\
print(variable.. \"EComponentS\")\n\
\
"
#endif

class LuaUtils{
public:
    static void CreateLuaECSystem(std::filesystem::path path);
    static void CreateLuaEComponenteS(std::filesystem::path path);
};