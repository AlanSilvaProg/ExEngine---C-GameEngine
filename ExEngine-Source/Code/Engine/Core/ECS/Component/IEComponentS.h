#pragma once 
#include "IEComponentSBase.h"
#include "EComponentSPropertyData.h"
#include "../../Serializer/ExSerializedClass.h"
#include <vector>

struct IEComponentS : public IEComponentSBase{
protected:
    static unsigned int previousId;
public:
    virtual ~IEComponentS() = default;
    virtual ExSerializedClass Serialize() override { return ExSerializedClass{}; };
};