#ifndef VEXORE_FEATUREHANDLERS_H
#define VEXORE_FEATUREHANDLERS_H

#include "../core/Types.h"

void FeatureHandlerEsp(const struct EspConfig* esp);
void FeatureHandlerAim(const struct AimConfig* aim);
void FeatureHandlerSkillCheck(const struct SkillCheckConfig* sc);
void FeatureHandlerSpeedHack(const struct SpeedHackConfig* sh);
void FeatureHandlerAura(const struct AuraConfig* aura);
void FeatureHandlersDispatch(const struct CheatConfig* cfg);

#endif
