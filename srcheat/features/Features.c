#include "Features.h"
#include "../core/Globals.h"
#include "GameRuntime.h"
#include "Esp.h"
#include "Aimbot.h"
#include "SkillCheck.h"
#include "SpeedHack.h"
#include "Aura.h"

void FeatureApplyAll(const struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    EspOverlayApply(&cfg->esp);
    AimLogicApply(&cfg->aim);
    SkillCheckApply(&cfg->skillCheck);
    SpeedHackApply(&cfg->speedHack);
    AuraApply(&cfg->aura);
}

void FeatureRestoreAll(void)
{
    AimClearLock();
    SkillCheckRestore();
    SpeedHackRestore();
    AuraRestore();
}
