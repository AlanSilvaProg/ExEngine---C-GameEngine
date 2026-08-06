#pragma once
// Single entry point for user scripts (project Assets), analogous to Unity's "using UnityEngine;" -
// pulls in everything needed to author a Component (EComponentS<T>) or a System (CustomECSystem):
// registration macros (REGISTER_COMPONENT/REGISTER_SYSTEM), ECSManager/EntityCS, and Logger.
#include "Core/ECS/InternalRegistry/ComponentRegistry.h"
#include "Core/ECS/InternalRegistry/SystemRegistry.h"
#include "Logger/Logger.h"
