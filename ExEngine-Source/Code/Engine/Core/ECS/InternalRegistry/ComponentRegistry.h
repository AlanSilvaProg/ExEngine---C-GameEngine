#pragma once
#include "../Component/IEComponentS.h"
#include "../ECSManager.h"
#include "../../Serializer/Demangle.h"
#include <memory>
#include <unordered_map>

class ComponentRegistry{
public:
    static inline std::unordered_map<unsigned int, std::function<void(std::shared_ptr<EntityCS>)>> components;
    static inline std::unordered_map<unsigned int, std::function<void(std::shared_ptr<CustomECSystem>, const bool)>> ecsystemRequirement;
    static inline std::unordered_map<unsigned int, std::string> componentsNameById;
    static inline std::unordered_map<unsigned int, std::function<void(std::shared_ptr<IEComponentS>&)>> componentFactory;
    static inline std::unordered_map<unsigned int, std::function<void(std::shared_ptr<IEComponentS>&, std::shared_ptr<IEComponentS>&)>> componentCloneFactory;

    // Removes every entry registered for a componentId. Required before unloading a hot-reloaded
    // script module: the factories captured here point at code from that module's translation unit,
    // and become dangling as soon as the module is dlclose'd/FreeLibrary'd.
    static inline void Unregister(const unsigned int componentId){
        components.erase(componentId);
        ecsystemRequirement.erase(componentId);
        componentsNameById.erase(componentId);
        componentFactory.erase(componentId);
        componentCloneFactory.erase(componentId);
    };
};

#ifndef REGISTER_COMPONENT
#define REGISTER_COMPONENT(type)\
namespace{\
struct type##AutoRegister{\
    static void Register(){\
        static bool registered = false;\
        if(!registered)\
        {\
            ComponentRegistry::components.insert({type::GetId(), [](std::shared_ptr<EntityCS> entity){ entity->AddComponent<type>(); }});\
            ComponentRegistry::ecsystemRequirement.insert({type::GetId(), [](std::shared_ptr<CustomECSystem> ecsystem, const bool isOptional){ ecsystem->AddRequire<type>(isOptional); }});\
            ComponentRegistry::componentsNameById.insert({type::GetId(), Demangle(typeid(type).name())});\
            ComponentRegistry::componentFactory.insert({type::GetId(), [](std::shared_ptr<IEComponentS>& component){ component = std::make_shared<type>(); }});\
            ComponentRegistry::componentCloneFactory.insert({type::GetId(), [](std::shared_ptr<IEComponentS>& component, std::shared_ptr<IEComponentS>& cloneComponent){ cloneComponent = std::make_shared<type>(*std::static_pointer_cast<type>(component)); }});\
            registered = true;\
        }\
    }    \
};\
\
static bool global_##type##registered = (type##AutoRegister::Register(), true);\
};
#endif