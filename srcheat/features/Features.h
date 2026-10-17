#ifndef VEXORE_FEATURES_H
#define VEXORE_FEATURES_H

#include "../core/Types.h"
#include "Esp.h"
#include "Aimbot.h"
#include "SkillCheck.h"
#include "SpeedHack.h"
#include "Aura.h"

void FeatureApplyAll(const struct CheatConfig* cfg);
void FeatureRestoreAll(void);

#endif
