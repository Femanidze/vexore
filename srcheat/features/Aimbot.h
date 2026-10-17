#ifndef VEXORE_AIMBOT_H
#define VEXORE_AIMBOT_H

#include "../core/Types.h"
#include "GameRuntime.h"

void AimLogicApply(const struct AimConfig* cfg);
int AimInFov(const struct Vec2* p, float radius);
void AimClearLock(void);

#endif
