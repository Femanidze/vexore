#ifndef VEXORE_AURA_H
#define VEXORE_AURA_H

#include "../core/Types.h"
#include "GameRuntime.h"

void AuraApply(const struct AuraConfig* aura);
void AuraRestore(void);
int AuraWithin(const struct AuraToggle* tog, float dist);
int AuraEntityFlagged(const struct AuraConfig* aura, const struct EspTargetState* t);
const struct AuraToggle* AuraSelectToggle(const struct AuraConfig* aura, const struct EspTargetState* t);

#endif
