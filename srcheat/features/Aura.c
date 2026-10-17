#include "Aura.h"
#include "../core/Globals.h"
#include "../core/Offsets.h"
#include "../core/LocalPlayer.h"
#include "GameRuntime.h"

#include <string.h>

static struct {
    qword addr;
    byte original;
    int tracked;
} s_auraTracked[16];
static int s_auraTrackedCount;

static void AuraTrackByte(qword addr, byte val)
{
    byte orig = 0;
    int i;

    if (!IsValidPtr(addr))
        return;
    for (i = 0; i < s_auraTrackedCount; i++) {
        if (s_auraTracked[i].addr == addr) {
            GameSafeWrite(addr, &val, 1);
            return;
        }
    }
    if (s_auraTrackedCount >= 16)
        return;
    if (!GameSafeRead(addr, &orig, 1))
        return;
    s_auraTracked[s_auraTrackedCount].addr = addr;
    s_auraTracked[s_auraTrackedCount].original = orig;
    s_auraTracked[s_auraTrackedCount].tracked = 1;
    s_auraTrackedCount++;
    GameSafeWrite(addr, &val, 1);
}

static void AuraRestoreAll(void)
{
    int i;
    for (i = 0; i < s_auraTrackedCount; i++) {
        if (IsValidPtr(s_auraTracked[i].addr))
            GameSafeWrite(s_auraTracked[i].addr, &s_auraTracked[i].original, 1);
    }
    s_auraTrackedCount = 0;
}

int AuraWithin(const struct AuraToggle* tog, float dist)
{
    if (!tog || !tog->enabled)
        return 0;
    if (tog->distance > 0.0f && dist > tog->distance)
        return 0;
    return 1;
}

const struct AuraToggle* AuraSelectToggle(const struct AuraConfig* aura, const struct EspTargetState* t)
{
    if (!aura || !t)
        return 0;
    switch (t->entityType) {
    case EntHatch:
        if (AuraWithin(&aura->hatch, t->distance))
            return &aura->hatch;
        break;
    case EntHook:
        if (AuraWithin(&aura->hooks, t->distance))
            return &aura->hooks;
        break;
    case EntChest:
        if (AuraWithin(&aura->chests, t->distance))
            return &aura->chests;
        break;
    case EntTotem:
        if (AuraWithin(&aura->totems, t->distance))
            return &aura->totems;
        break;
    case EntPallet:
        if (AuraWithin(&aura->pallets, t->distance))
            return &aura->pallets;
        break;
    default:
        if (AuraWithin(&aura->hatch, t->distance) && t->name[0] && strstr(t->name, "Hatch"))
            return &aura->hatch;
        if (AuraWithin(&aura->hooks, t->distance) && t->name[0] && strstr(t->name, "Hook"))
            return &aura->hooks;
        if (AuraWithin(&aura->chests, t->distance) && t->name[0] && strstr(t->name, "Chest"))
            return &aura->chests;
        if (AuraWithin(&aura->totems, t->distance) && t->name[0] && strstr(t->name, "Totem"))
            return &aura->totems;
        if (AuraWithin(&aura->pallets, t->distance) && t->name[0] && strstr(t->name, "Pallet"))
            return &aura->pallets;
        break;
    }
    return 0;
}

int AuraEntityFlagged(const struct AuraConfig* aura, const struct EspTargetState* t)
{
    if (!aura || !t)
        return 0;
    if (t->distance <= 0.0f)
        return 0;
    if (t->entityType == EntGenerator || t->entityType == EntWindow || t->entityType == EntExit)
        return aura->masterSwitch;
    if (AuraSelectToggle(aura, t))
        return 1;
    if (aura->trapsMaster && t->distance <= aura->trapsDistance) {
        if (aura->trapBear && t->entityType == EntTrap)
            return 1;
        if (aura->trapZombie && t->entityType == EntTrap)
            return 1;
        if (t->name[0] && strstr(t->name, "Trap"))
            return 1;
    }
    return 0;
}

static void AuraPatchEntity(const struct AuraConfig* aura, struct EspTargetState* t)
{
    qword actor;
    byte hidden = 0;

    if (!aura || !t || !t->actorAddr)
        return;
    actor = t->actorAddr;
    if (!IsValidPtr(actor))
        return;
    if (t->entityType == EntHatch || t->entityType == EntTotem || t->entityType == EntPallet ||
        t->entityType == EntChest || t->entityType == EntHook || t->entityType == EntTrap) {
        if (GameSafeRead(actor + 0x7C8, &hidden, 1)) {
            if (aura->masterSwitch)
                AuraTrackByte(actor + 0x7C8, 0);
        }
        if (t->entityType == EntTotem) {
            byte stA = 0;
            if (GameSafeRead(actor + Off_TotemStateA, &stA, 1))
                AuraTrackByte(actor + Off_TotemStateA, stA);
        }
        if (t->entityType == EntPallet) {
            byte st = 0;
            if (GameSafeRead(actor + Off_PalletState, &st, 1))
                AuraTrackByte(actor + Off_PalletState, st);
        }
    }
    if (t->entityType == EntGenerator) {
        float done = 0.0f;
        if (GameSafeRead(actor + Off_ActorProgress, &done, 4) && done >= 0.0f && done <= ProgressMax)
            t->progressPct = done;
    }
}

void AuraApply(const struct AuraConfig* aura)
{
    int i;

    if (!aura || !aura->masterSwitch) {
        AuraRestoreAll();
        return;
    }
    __try {
        if (!g_connected || !g_gameBase)
            return;
        for (i = 0; i < g_runtime.targetCount; i++) {
            struct EspTargetState* t = &g_runtime.targets[i];
            if (AuraEntityFlagged(aura, t)) {
                t->visible = 1;
                AuraPatchEntity(aura, t);
            }
        }
    } __except (1) {
    }
}

void AuraRestore(void)
{
    AuraRestoreAll();
}
