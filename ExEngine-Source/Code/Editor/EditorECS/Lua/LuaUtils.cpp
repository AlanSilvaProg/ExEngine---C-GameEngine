#include "LuaUtils.h"
#include "../../../Engine/File/FileManagement.h"
#include "../../Main/EditorInterfaceGetters.h"
#include "../../../Engine/Logger/Logger.h"

void LuaUtils::CreateLuaECSystem(std::filesystem::path path){
    if(FileManagement::CreateFile(path, BASE_LUA_ECSYSTEM))
        RegisterLua(path);
};

void LuaUtils::CreateLuaEComponenteS(std::filesystem::path path){
    if(FileManagement::CreateFile(path, BASE_LUA_ECOMPONENTS))
        RegisterLua(path);
};

void LuaUtils::RegisterLua(const std::filesystem::path& path){
    auto& luaState = *EditorInterfaceGetters::engine->GetSolState();
    auto luaResult = luaState.load_file(path.string());
    sol::table resultType = luaResult();

    for(auto e : resultType)
    {
        Logger::LogError("Aqui: " + e.first.as<std::string>()); // printing fields
    }

    //ToDo Criar componentes através do lua, registrar novo tipo de componente no ecs manager
    // Usar File Watcher para identificar alterações no arquivo ( revalidar - hot reload ) 
    // Adicionar componentes no inspector 
};