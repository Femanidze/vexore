#include "FeatureHandlers.h"
#include "../core/Globals.h"
#include "Esp.h"
#include "Aimbot.h"
#include "SkillCheck.h"
#include "SpeedHack.h"
#include "Aura.h"
#include "../render/OverlayRender.h"

void FeatureHandlerEsp(const struct EspConfig* esp)
{
    if (!esp || !esp->enabled)
        return;
    EspOverlayApply(esp);
    OverlayDrawEsp(esp);
}

void FeatureHandlerAim(const struct AimConfig* aim)
{
    if (!aim || !aim->enabled)
        return;
    AimLogicApply(aim);
}

void FeatureHandlerSkillCheck(const struct SkillCheckConfig* sc)
{
    if (!sc || !sc->enabled)
        return;
    SkillCheckApply(sc);
}

void FeatureHandlerSpeedHack(const struct SpeedHackConfig* sh)
{
    if (!sh || !sh->enabled)
        return;
    SpeedHackApply(sh);
}

void FeatureHandlerAura(const struct AuraConfig* aura)
{
    if (!aura || !aura->masterSwitch)
        return;
    AuraApply(aura);
    OverlayDrawAura(aura);
}

void FeatureHandlersDispatch(const struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    FeatureHandlerEsp(&cfg->esp);
    FeatureHandlerAim(&cfg->aim);
    FeatureHandlerSkillCheck(&cfg->skillCheck);
    FeatureHandlerSpeedHack(&cfg->speedHack);
    FeatureHandlerAura(&cfg->aura);
    OverlayDrawCrosshair(cfg);
    OverlayDrawWatermark(cfg);
}
