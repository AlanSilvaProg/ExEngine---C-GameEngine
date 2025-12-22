#pragma once
#include <filesystem>
#include <sol/sol.hpp>
#include <memory>

#ifndef BASE_LUA_ECSYSTEM
#define BASE_LUA_ECSYSTEM "\
--Avoid changes to this id \n\
id += 1 \n\
\n\
print(\"ECSystem created with id: \" .. id)\n\
\
"
#endif

#ifndef BASE_LUA_ECOMPONENTS
#define BASE_LUA_ECOMPONENTS "\
return {\n\
    id = \"ComponentName\", \n\
    fields = {\n\
        field = 0\n\
    }\n\
}\n\
\
"
#endif

class LuaUtils{
private:
    std::shared_ptr<sol::state> solState;

    static void RegisterLua(const std::filesystem::path& path);
public:
    static void CreateLuaECSystem(std::filesystem::path path);
    static void CreateLuaEComponenteS(std::filesystem::path path);
};