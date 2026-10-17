#ifndef VEXORE_SPEEDHACK_H
#define VEXORE_SPEEDHACK_H

#include "../core/Types.h"

void SpeedHackApply(const struct SpeedHackConfig* cfg);
void SpeedHackRestore(void);
float SpeedHackClampValue(float v);
float SpeedHackScaleDelta(float delta, const struct SpeedHackConfig* cfg);

#endif
