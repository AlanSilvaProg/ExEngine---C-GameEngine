#pragma once
#include <sol/sol.hpp>

class ScriptingExecutor{
private:
    sol::state luaState;

    int earlyCallbackId;
    int updateCallbackId;
    int fixedUpdateCallbackId;
    int lateCallbackId;

    void EarlyUpdateExecution();
    void UpdateExecution();
    void FixedUpdateExecution();
    void LateUpdateExecution();
public:
    ScriptingExecutor();
    ~ScriptingExecutor();
};