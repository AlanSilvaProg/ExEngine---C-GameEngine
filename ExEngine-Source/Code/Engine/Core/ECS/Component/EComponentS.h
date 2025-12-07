#pragma once
#include "IEComponentS.h"
#include "../../Serializer/Demangle.h"
#include <typeinfo>

template<typename T>
struct EComponentS : public IEComponentS {
public:
    static unsigned int GetId() {
        static unsigned int id = previousId++;
        return id;
    }

    virtual int GetComponentId() override { return GetId(); }
    virtual ExSerializedClass Serialize() override { return ExSerializedClass{}; }
    virtual std::string GetComponentName() override { return StaticGetComponentName(); }

    inline static std::string StaticGetComponentName() { return Demangle(typeid(T).name()); }
};