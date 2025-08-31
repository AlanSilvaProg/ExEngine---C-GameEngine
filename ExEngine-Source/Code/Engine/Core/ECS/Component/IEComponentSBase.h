#pragma once
#include "../../Serializer/ExEngineSerializer.h"
#include "../../Serializer/ISerializable.h"

struct IEComponentSBase : public ISerializable
{
protected:
    static unsigned int previousId;
public:
    virtual int GetComponentId() = 0;
};