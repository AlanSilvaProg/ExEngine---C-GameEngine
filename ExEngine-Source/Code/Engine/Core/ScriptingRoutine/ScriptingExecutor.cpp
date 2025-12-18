#include "ScriptingExecutor.h"
#include "../../GameCore/Runtime/RuntimeEvent/GameUpdateEventHandler.h"

ScriptingExecutor::ScriptingExecutor(){
    earlyCallbackId = *GameUpdateEventHandler::earlyHandler += [this](){ EarlyUpdateExecution(); };
    updateCallbackId = *GameUpdateEventHandler::updateHandler += [this](){ UpdateExecution(); };
    fixedUpdateCallbackId = *GameUpdateEventHandler::fixedUpdateHandler += [this](){ FixedUpdateExecution(); };
    lateCallbackId = *GameUpdateEventHandler::lateHandler += [this](){ LateUpdateExecution(); };
};

void ScriptingExecutor::EarlyUpdateExecution(){
    //ToDo execute registered lua scripts that inherit from ExBehavior class
};

void ScriptingExecutor::UpdateExecution(){
    //ToDo execute registered lua scripts that inherit from ExBehavior class
};

void ScriptingExecutor::FixedUpdateExecution(){
    //ToDo execute registered lua scripts that inherit from ExBehavior class
};

void ScriptingExecutor::LateUpdateExecution(){
    //ToDo execute registered lua scripts that inherit from ExBehavior class
};

ScriptingExecutor::~ScriptingExecutor(){
    *GameUpdateEventHandler::earlyHandler -= earlyCallbackId;
    *GameUpdateEventHandler::updateHandler -= updateCallbackId;
    *GameUpdateEventHandler::fixedUpdateHandler -= fixedUpdateCallbackId;
    *GameUpdateEventHandler::lateHandler -= lateCallbackId;
};