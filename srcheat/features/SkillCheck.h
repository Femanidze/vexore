#ifndef VEXORE_SKILLCHECK_H
#define VEXORE_SKILLCHECK_H

#include "../core/Types.h"

void SkillCheckApply(const struct SkillCheckConfig* cfg);
void SkillCheckRestore(void);
float SkillCheckClampGreat(float v);
int SkillCheckZoneValid(int zoneMode);
int SkillCheckShouldAutoGreat(const struct SkillCheckConfig* cfg, float roll);

#endif
