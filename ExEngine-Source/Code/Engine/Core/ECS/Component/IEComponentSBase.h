#pragma once
#include "../../Serializer/ExEngineSerializer.h"
#include "../../Serializer/ISerializable.h"

struct IEComponentSBase : public ISerializable
{
public:
    virtual int GetComponentId() = 0;
    virtual std::string GetComponentName() = 0;
};