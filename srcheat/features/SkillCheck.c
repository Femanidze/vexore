#include "SkillCheck.h"
#include "../core/Globals.h"
#include "../core/Offsets.h"
#include "../core/LocalPlayer.h"
#include "GameRuntime.h"

#include <windows.h>
#include <string.h>

static int s_skillActive;
static DWORD s_lastGreatPress;
static struct {
    qword addr;
    float original;
    int tracked;
} s_skillZone;

float SkillCheckClampGreat(float v)
{
    if (v < 0.0f)
        return 0.0f;
    if (v > 100.0f)
        return 100.0f;
    return v;
}

int SkillCheckZoneValid(int zoneMode)
{
    return zoneMode >= SkillCheckDefaultZones && zoneMode <= SkillCheckWideGreat;
}

int SkillCheckShouldAutoGreat(const struct SkillCheckConfig* cfg, float roll)
{
    if (!cfg || !cfg->enabled)
        return 0;
    if (!SkillCheckZoneValid(cfg->zoneMode))
        return 0;
    if (cfg->greatChance <= 0.0f)
        return 0;
    return roll < cfg->greatChance;
}

static void SkillCheckTrackZone(qword addr, float val)
{
    float orig = 0.0f;

    if (!IsValidPtr(addr))
        return;
    if (!s_skillZone.tracked || s_skillZone.addr != addr) {
        if (GameSafeRead(addr, &orig, 4)) {
            s_skillZone.addr = addr;
            s_skillZone.original = orig;
            s_skillZone.tracked = 1;
        }
    }
    GameSafeWrite(addr, &val, 4);
}

static void SkillCheckRestoreZone(void)
{
    if (!s_skillZone.tracked)
        return;
    if (IsValidPtr(s_skillZone.addr))
        GameSafeWrite(s_skillZone.addr, &s_skillZone.original, 4);
    s_skillZone.tracked = 0;
    s_skillZone.addr = 0;
    s_skillZone.original = 0.0f;
}

static int SkillCheckReadActive(void)
{
    qword actor;
    float health = 0.0f;
    float progress = 0.0f;
    float recovery = 0.0f;
    byte interact = 0;

    actor = LocalPlayerActor();
    if (!IsValidPtr(actor))
        return 0;
    if (!GameReadActorHealth(actor, &health, &progress, &recovery))
        return 0;
    if (GameSafeRead(actor + Off_ActorFlagByte, &interact, 1)) {
        if (interact & 0x20)
            return 1;
        if (interact & 0x40)
            return 1;
    }
    if (health > 0.0f && progress > 0.0f && progress < 0.999f)
        return 1;
    return 0;
}

static int SkillCheckPatchGreat(qword actor, const struct SkillCheckConfig* cfg)
{
    float great = 1.0f;
    float cur = 0.0f;

    if (!IsValidPtr(actor))
        return 0;
    if (cfg->zoneMode == SkillCheckPerfectOnly || cfg->zoneMode == SkillCheckWideGreat) {
        if (GameSafeRead(actor + Off_ActorProgress, &cur, 4) && cur >= 0.0f && cur <= 1.0f) {
            SkillCheckTrackZone(actor + Off_ActorProgress, great);
            return 1;
        }
        if (GameSafeRead(actor + Off_ActorRecovery, &cur, 4) && cur >= 0.0f && cur <= 1.0f) {
            SkillCheckTrackZone(actor + Off_ActorRecovery, great);
            return 1;
        }
    }
    return 0;
}

static void SkillCheckForceGreat(const struct SkillCheckConfig* cfg)
{
    DWORD now;
    qword actor;

    now = GetTickCount();
    if (now - s_lastGreatPress < 80)
        return;
    s_lastGreatPress = now;
    actor = LocalPlayerActor();
    SkillCheckPatchGreat(actor, cfg);
    keybd_event(0x20, 0, 0, 0);
    keybd_event(0x20, 0, 2, 0);
}

void SkillCheckApply(const struct SkillCheckConfig* cfg)
{
    int active;

    if (!cfg || !cfg->enabled) {
        s_skillActive = 0;
        SkillCheckRestoreZone();
        return;
    }
    if (!SkillCheckZoneValid(cfg->zoneMode))
        return;
    __try {
        g_config.skillCheck.greatChance = SkillCheckClampGreat(cfg->greatChance);
        if (!g_connected || !g_gameBase)
            return;
        LocalPlayerResolve();
        active = SkillCheckReadActive();
        if (active) {
            float roll = 100.0f - cfg->greatChance;
            if (SkillCheckShouldAutoGreat(cfg, roll))
                SkillCheckForceGreat(cfg);
        }
        s_skillActive = active;
    } __except (1) {
        s_skillActive = 0;
    }
}

void SkillCheckRestore(void)
{
    s_skillActive = 0;
    s_lastGreatPress = 0;
    SkillCheckRestoreZone();
}
