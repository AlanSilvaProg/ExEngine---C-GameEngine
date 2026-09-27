#pragma once
#include "../ECSManager.h"
#include "../../Serializer/Demangle.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <utility>
#include <string>

// Analogous to ComponentRegistry, but for hot-reloaded ECSystem scripts: each reload gives "the
// same" class a new type_info, so systems are keyed by a stable, user-assigned SystemId instead.
// Registering only stores a factory; ScriptHotReloadManager instantiates it against the live
// ECSManager, returning (type_index, instance) so the caller can tear it down on the next reload.
class SystemRegistry{
public:
    static inline std::unordered_map<unsigned int, std::function<std::pair<std::type_index, std::shared_ptr<ECSystem>>(std::shared_ptr<ECSManager>)>> systemFactories;
    static inline std::unordered_map<unsigned int, std::string> systemsNameById;
    // The SystemContext each systemId's script declared itself with (REGISTER_SYSTEM's 2nd argument).
    // Used by the editor to offer moving a freshly-added system straight into a different step.
    static inline std::unordered_map<unsigned int, SystemContext> systemContextById;

    // Removes the factory registered for a systemId. Required before unloading a hot-reloaded
    // script module: the factory captured here points at code from that module's translation unit,
    // and becomes dangling as soon as the module is dlclose'd/FreeLibrary'd.
    static inline void Unregister(const unsigned int systemId){
        systemFactories.erase(systemId);
        systemsNameById.erase(systemId);
        systemContextById.erase(systemId);
    };
};

#ifndef REGISTER_SYSTEM
#define REGISTER_SYSTEM(type, systemContext)\
namespace{\
struct type##SystemAutoRegister{\
    static void Register(){\
        static bool registered = false;\
        if(!registered)\
        {\
            SystemRegistry::systemFactories.insert({type::SystemId, [](std::shared_ptr<ECSManager> ecsManager) -> std::pair<std::type_index, std::shared_ptr<ECSystem>> {\
                auto system = ecsManager->CreateSystem<type>();\
                auto typeIndex = std::type_index(typeid(type));\
                if(system != nullptr)\
                {\
                    ecsManager->GetECSystemContext(systemContext)->Register(typeIndex, system);\
                }\
                return { typeIndex, system };\
            }});\
            SystemRegistry::systemsNameById.insert({type::SystemId, Demangle(typeid(type).name())});\
            SystemRegistry::systemContextById.insert({type::SystemId, systemContext});\
            registered = true;\
        }\
    }    \
};\
\
static bool global_##type##systemregistered = (type##SystemAutoRegister::Register(), true);\
};
#endif
