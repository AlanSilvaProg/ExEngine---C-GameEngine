#pragma once
#include "IEComponentS.h"

template<typename T>
struct EComponentS : public IEComponentS{
public:
    static unsigned int GetId(){
        static auto id = previousId++;
        return id;
    };

    virtual int GetComponentId() override { return GetId(); };
    virtual ExSerializedClass Serialize() override { return ExSerializedClass{}; };
};