#pragma once
#include "IEComponentS.h"

template<typename T>
struct EComponentS : public IEComponentS{
public:
    static unsigned int GetId(){
        static auto id = previousId++;
        return id;
    };

    //virtual std::vector<EComponentSPropertyData> GetPropertyData() override = 0;
};