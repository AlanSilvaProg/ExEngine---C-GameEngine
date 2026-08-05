#pragma once
#include "../ECSManager.h"
#include "../../Serializer/Demangle.h"
#include <memory>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <utility>
#include <string>

// Analogous to ComponentRegistry, but for strong-typed ECSystem subclasses defined in hot-reloaded
// scripts. Systems can't be keyed by std::type_index like ECSManager::systems does internally:
// every dlopen/LoadLibrary of a recompiled script produces a brand new type_info for "the same"
// class, so a stable, user-assigned SystemId (mirroring EComponentS::ComponentId) is used instead.
//
// Registering here does NOT instantiate the system - it only stores a factory. Actual instantiation
// happens later, driven by ScriptHotReloadManager, which is the only place with access to the live
// ECSManager. This keeps the Engine free of any dependency on Editor-only globals. The factory
// returns the (type_index, instance) pair it created so the caller can tear it down again on the
// next reload (ECSManager::DestroySystem/ECSystemContext::Unregister both need that pair).
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
