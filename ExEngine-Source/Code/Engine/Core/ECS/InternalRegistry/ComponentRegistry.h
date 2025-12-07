#pragma once
#include "../Component/IEComponentS.h"
#include "../ECSManager.h"
#include "../../Serializer/Demangle.h"
#include <memory>
#include <unordered_map>

class ComponentRegistry{
public:
    static inline std::unordered_map<unsigned int, std::function<void(std::shared_ptr<EntityCS>)>> components;
    static inline std::unordered_map<unsigned int, std::string> componentsNameById;
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
            ComponentRegistry::componentsNameById.insert({type::GetId(), Demangle(typeid(type).name())});\
            registered = true;\
        }\
    }    \
};\
\
static bool global_##type##registered = (type##AutoRegister::Register(), true);\
};
#endif