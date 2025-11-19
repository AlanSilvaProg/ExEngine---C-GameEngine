#pragma once
#include "Engine.h"
#include <memory>

class EngineGetters{
private:
    static inline std::shared_ptr<Engine> engine;

public:
    inline EngineGetters(std::shared_ptr<Engine> enginePtr) { engine = enginePtr; };

    static inline const std::shared_ptr<Engine> GetEnginePtr() { return engine; };
};