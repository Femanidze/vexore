#include "GameRuntime.h"
#include "../core/Globals.h"
#include "../core/Offsets.h"
#include "../core/LocalPlayer.h"
#include "Features.h"
#include "Esp.h"
#include "Aimbot.h"
#include "SkillCheck.h"
#include "SpeedHack.h"
#include "Aura.h"
#include "../core/Timing.h"

#include <windows.h>
#include <string.h>

struct GameRuntimeState g_runtime;

#define GAME_MIN_PTR 0x10000ULL
#define GAME_MAX_PTR 0x7FFFFEFFFFFFULL
#define GAME_SAFE_END 0x7FFFFFFEFFFFULL

int IsValidPtr(qword addr)
{
    if (addr < GAME_MIN_PTR)
        return 0;
    if (addr - GAME_MIN_PTR > GAME_MAX_PTR - GAME_MIN_PTR)
        return 0;
    if (addr - 0x1000000ULL > GAME_SAFE_END - 0x1000000ULL)
        return 0;
    return 1;
}

int GameSafeRead(qword addr, void* out, unsigned int size)
{
    if (!out || size == 0)
        return 0;
    if (!g_connected)
        return 0;
    if (!IsValidPtr(addr))
        return 0;
    if (addr + size < addr)
        return 0;
    if (!IsValidPtr(addr + size - 1))
        return 0;
    __try {
        memcpy(out, (const void*)(uintptr_t)addr, size);
    } __except (1) {
        return 0;
    }
    return 1;
}

int GameSafeWrite(qword addr, const void* in, unsigned int size)
{
    unsigned long oldProt = 0;

    if (!in || size == 0)
        return 0;
    if (!g_connected)
        return 0;
    if (!IsValidPtr(addr))
        return 0;
    if (addr + size < addr)
        return 0;
    if (!IsValidPtr(addr + size - 1))
        return 0;
    __try {
        VirtualProtect((void*)(uintptr_t)addr, size, 0x40, &oldProt);
        memcpy((void*)(uintptr_t)addr, in, size);
        VirtualProtect((void*)(uintptr_t)addr, size, oldProt, &oldProt);
    } __except (1) {
        return 0;
    }
    return 1;
}

qword GameResolveRoot(void)
{
    qword addr;
    qword root = 0;

    if (!g_gameBase || !g_connected)
        return 0;
    if (g_signatureOffA && g_signatureOffA >= g_gameBase)
        addr = g_signatureOffA;
    else if (g_signatureOffA)
        addr = g_gameBase + g_signatureOffA;
    else
        addr = g_gameBase;
    if (!IsValidPtr(addr))
        return 0;
    if (!GameSafeRead(addr, &root, 8))
        return 0;
    if (!IsValidPtr(root))
        return 0;
    return root;
}

qword GameResolvePlayerArray(unsigned int* outCount)
{
    qword root;
    qword mid = 0;
    qword list = 0;
    unsigned int count = 0;

    if (outCount)
        *outCount = 0;
    root = GameResolveRoot();
    if (!root)
        return 0;
    if (!GameSafeRead(root + Off_WorldToMid, &mid, 8))
        return 0;
    if (!IsValidPtr(mid))
        return 0;
    if (!GameSafeRead(mid + Off_MidPlayerArray, &list, 8))
        return 0;
    if (!IsValidPtr(list))
        return 0;
    if (!GameSafeRead(mid + Off_MidPlayerCount, &count, 4))
        return 0;
    if (count == 0 || count > Off_PlayerListCountMax)
        return 0;
    if (outCount)
        *outCount = count;
    g_runtime.midAddr = mid;
    g_runtime.playerArray = list;
    g_runtime.playerCount = count;
    return list;
}

qword GameResolveEntityArray(unsigned int* outCount)
{
    qword root;
    qword container = 0;
    qword list = 0;
    unsigned int count = 0;

    if (outCount)
        *outCount = 0;
    root = GameResolveRoot();
    if (!root)
        return 0;
    g_runtime.rootAddr = root;
    if (!GameSafeRead(root + Off_WorldToEntities, &container, 8))
        return 0;
    if (!IsValidPtr(container))
        return 0;
    if (!GameSafeRead(container + Off_EntityListArray, &list, 8))
        return 0;
    if (!IsValidPtr(list))
        return 0;
    if (!GameSafeRead(container + Off_EntityListCount, &count, 4))
        return 0;
    if (count < Off_EntityListCountMin || count > Off_EntityListCountMax)
        return 0;
    if (outCount)
        *outCount = count;
    g_runtime.entityArray = list;
    g_runtime.entityCount = count;
    return list;
}

int GameClassifyName(const char* name)
{
    if (!name || !name[0])
        return EntNone;
    if (strstr(name, "Generator") || strstr(name, "GEN"))
        return EntGenerator;
    if (strstr(name, "Hatch") || strstr(name, "BP_Hatch01_C"))
        return EntHatch;
    if (strstr(name, "Totem") || strstr(name, "HexTotem"))
        return EntTotem;
    if (strstr(name, "Pallet") || strstr(name, "PALLET"))
        return EntPallet;
    if (strstr(name, "Chest"))
        return EntChest;
    if (strstr(name, "Hook"))
        return EntHook;
    if (strstr(name, "Window") || strstr(name, "EscapeBlocker"))
        return EntWindow;
    if (strstr(name, "BearTrap") || strstr(name, "ZombieBearTrap") || strstr(name, "Trap"))
        return EntTrap;
    if (strstr(name, "EXIT") || strstr(name, "EscapeDoor"))
        return EntExit;
    return EntNone;
}

int GameReadActorPosition(qword actor, struct Vec3* out)
{
    qword component = 0;

    if (!out)
        return 0;
    memset(out, 0, sizeof(*out));
    if (!IsValidPtr(actor))
        return 0;
    if (!GameSafeRead(actor + Off_ComponentOwner, &component, 8))
        return 0;
    if (!IsValidPtr(component))
        return 0;
    if (!GameSafeRead(component + Off_ComponentPosition, out, Off_ComponentPositionSize))
        return 0;
    if (out->x != out->x || out->y != out->y || out->z != out->z)
        return 0;
    return 1;
}

int GameReadActorHealth(qword actor, float* health, float* progress, float* recovery)
{
    float h = 0.0f;
    float p = 0.0f;
    float r = 0.0f;

    if (health)
        *health = 0.0f;
    if (progress)
        *progress = 0.0f;
    if (recovery)
        *recovery = 0.0f;
    if (!IsValidPtr(actor))
        return 0;
    GameSafeRead(actor + Off_ActorHealth, &h, 4);
    GameSafeRead(actor + Off_ActorProgress, &p, 4);
    GameSafeRead(actor + Off_ActorRecovery, &r, 4);
    if (p != p)
        p = 0.0f;
    if (p < 0.0f)
        p = 0.0f;
    if (p > 1.0f)
        p = 1.0f;
    if (health)
        *health = h;
    if (progress)
        *progress = p;
    if (recovery)
        *recovery = r;
    return 1;
}

static void GameReadName(qword entity, char* out, int outSize)
{
    int slot;
    size_t len = 0;

    if (!out || outSize <= 0)
        return;
    out[0] = 0;
    for (slot = 0; slot < Off_EntityNameSlotCount; slot++) {
        unsigned int nameId = 0;
        char part[32];
        if (!GameSafeRead(entity + Off_EntityNameSlotBase + (qword)slot * Off_EntityNameSlotStride, &nameId, 4))
            continue;
        if (nameId == 0 || nameId == 0xFFFFFFFF)
            continue;
        memset(part, 0, sizeof(part));
        part[0] = (char)(nameId & 0xFF);
        part[1] = (char)((nameId >> 8) & 0xFF);
        part[2] = (char)((nameId >> 16) & 0xFF);
        part[3] = (char)((nameId >> 24) & 0xFF);
        if (part[0]) {
            size_t n = strlen(part);
            if (len + n + 1 < (size_t)outSize) {
                memcpy(out + len, part, n);
                len += n;
                out[len] = 0;
            }
        }
    }
}

int GameReadEntityRecord(qword entity, struct EspTargetState* out)
{
    qword actor = 0;
    unsigned int type = 0;
    float health = 0.0f;
    float progress = 0.0f;
    float recovery = 0.0f;
    struct Vec3 pos;
    int cls;
    char rawName[64];

    if (!out || entity < GAME_MIN_PTR)
        return 0;
    memset(out, 0, sizeof(*out));
    if (!GameSafeRead(entity + Off_EntityType, &type, 4))
        return 0;
    out->entityType = (int)type;
    out->entityAddr = entity;
    if (!GameSafeRead(entity + Off_EntityActor, &actor, 8))
        return 0;
    if (!IsValidPtr(actor)) {
        if (type != Off_EntityTypeSurvivor)
            return 0;
        actor = entity;
    }
    out->actorAddr = actor;
    memset(rawName, 0, sizeof(rawName));
    GameReadName(entity, rawName, sizeof(rawName));
    cls = GameClassifyName(rawName);
    if (cls != EntNone)
        out->entityType = cls;
    if (rawName[0])
        strncpy(out->name, rawName, sizeof(out->name) - 1);
    memset(&pos, 0, sizeof(pos));
    if (GameReadActorPosition(actor, &pos)) {
        out->world = pos;
    } else {
        qword nested = 0;
        if (GameSafeRead(actor + Off_ActorNested, &nested, 8) && IsValidPtr(nested))
            GameReadActorPosition(nested, &out->world);
    }
    if (type == Off_EntityTypeSurvivor || cls == EntSurvivor || cls == EntNone) {
        GameReadActorHealth(actor, &health, &progress, &recovery);
        out->progressPct = (health / HealthMaxSurvivor + progress) * ProgressMax;
        if (out->progressPct > ProgressMax)
            out->progressPct = ProgressMax;
        if (out->progressPct < 0.0f)
            out->progressPct = 0.0f;
        out->recoveryPct = recovery;
        if (health <= 0.0f)
            out->healthState = 3;
        else if (health < 30.0f)
            out->healthState = 2;
        else if (health < HealthMaxSurvivor)
            out->healthState = 1;
        else
            out->healthState = 0;
        if (type == Off_EntityTypeSurvivor && !out->name[0])
            strncpy(out->name, "Survivor", sizeof(out->name) - 1);
    } else if (cls == EntGenerator) {
        float genP = 0.0f;
        GameSafeRead(actor + Off_ActorProgress, &genP, 4);
        if (genP > 1.0f)
            out->progressPct = genP;
        else
            out->progressPct = genP * ProgressMax;
        if (out->progressPct > ProgressMax)
            out->progressPct = ProgressMax;
        if (out->progressPct < 0.0f)
            out->progressPct = 0.0f;
        out->healthState = 0;
        if (!out->name[0])
            strncpy(out->name, "Generator", sizeof(out->name) - 1);
    } else {
        out->healthState = 0;
    }
    if (!out->world.x && !out->world.y && !out->world.z)
        return 0;
    out->visible = 0;
    out->distance = 0.0f;
    return 1;
}

int GameRuntimeProject(const struct Vec3* world, struct Vec2* out)
{
    return EspProjectTarget(world, out);
}

void GameRuntimeLerpTargets(void)
{
    int i;
    float k = g_config.esp.lerpSmooth;
    if (k <= 0.0f)
        k = 0.15f;
    if (k > 1.0f)
        k = 1.0f;
    for (i = 0; i < g_runtime.targetCount; i++) {
        struct EspTargetState* t = &g_runtime.targets[i];
        if (g_runtime.featureFrame <= 1) {
            t->screenLerp.x = t->screen.x;
            t->screenLerp.y = t->screen.y;
        } else {
            t->screenLerp.x = t->screenLerp.x + (t->screen.x - t->screenLerp.x) * k;
            t->screenLerp.y = t->screenLerp.y + (t->screen.y - t->screenLerp.y) * k;
        }
    }
}

void GameRuntimeSetTargets(const struct EspTargetState* list, int count)
{
    int n = count;
    if (!list || n <= 0) {
        g_runtime.targetCount = 0;
        return;
    }
    if (n > 64)
        n = 64;
    memcpy(g_runtime.targets, list, (size_t)n * sizeof(struct EspTargetState));
    g_runtime.targetCount = n;
}

static void GameCollectDistance(void)
{
    int i;
    struct Vec3 lp;
    int haveLocal = 0;

    memset(&lp, 0, sizeof(lp));
    if (LocalPlayerReadPosition(&lp))
        haveLocal = 1;
    else
        g_localPlayer.healthPct = LocalPlayerReadHealthPct();
    for (i = 0; i < g_runtime.targetCount; i++) {
        struct EspTargetState* t = &g_runtime.targets[i];
        float dx, dy, dz;
        if (haveLocal) {
            dx = t->world.x - lp.x;
            dy = t->world.y - lp.y;
            dz = t->world.z - lp.z;
        } else {
            dx = t->world.x;
            dy = t->world.y;
            dz = t->world.z;
        }
        t->distance = dx * dx + dy * dy + dz * dz;
        if (t->distance > 0.0f) {
            float d = t->distance;
            float sqrtApprox = d;
            int n;
            for (n = 0; n < 6; n++)
                sqrtApprox = 0.5f * (sqrtApprox + d / (sqrtApprox > 0.0001f ? sqrtApprox : 0.0001f));
            t->distance = sqrtApprox;
        }
    }
}

void GameFeatureTick(void)
{
    unsigned int count = 0;
    qword list;
    unsigned int i;

    if (!g_gameBase)
        return;
    __try {
        g_runtime.featureFrame++;
        g_runtime.cachedBase = g_gameBase;
        g_runtime.targetWidth = g_espWindowWidth;
        g_runtime.targetHeight = g_espWindowHeight;
        g_runtime.targetCount = 0;
        g_runtime.worldValid = 0;
        g_runtime.rootAddr = 0;
        LocalPlayerResolve();
        list = GameResolveEntityArray(&count);
        if (!list || count == 0) {
            list = GameResolvePlayerArray(&count);
            if (!list || count == 0) {
                GameRuntimeLerpTargets();
                EspOverlayApply(&g_config.esp);
                AimLogicApply(&g_config.aim);
                SkillCheckApply(&g_config.skillCheck);
                SpeedHackApply(&g_config.speedHack);
                AuraApply(&g_config.aura);
                return;
            }
        }
        g_runtime.worldValid = 1;
        if (count > 64)
            count = 64;
        for (i = 0; i < count; i++) {
            qword entity = 0;
            struct EspTargetState rec;
            if (!GameSafeRead(list + (qword)i * 8, &entity, 8))
                continue;
            if (!entity)
                continue;
            if (!IsValidPtr(entity))
                continue;
            if (!GameReadEntityRecord(entity, &rec))
                continue;
            if (g_runtime.targetCount >= 64)
                break;
            g_runtime.targets[g_runtime.targetCount] = rec;
            g_runtime.targetCount++;
        }
        GameCollectDistance();
        GameRuntimeLerpTargets();
        EspOverlayApply(&g_config.esp);
        AimLogicApply(&g_config.aim);
        SkillCheckApply(&g_config.skillCheck);
        SpeedHackApply(&g_config.speedHack);
        AuraApply(&g_config.aura);
    } __except (1) {
    }
}

void SignatureScanTick(unsigned int moduleHandle, unsigned int width)
{
    if (!g_gameBase || moduleHandle == 0)
        return;
    g_runtime.signatureFrame++;
    if (width)
        g_espWindowWidth = width;
    if (g_signatureOffA == 0)
        g_signatureOffA = g_gameBase + 0x1000;
    if (g_signatureOffB == 0)
        g_signatureOffB = g_gameBase + 0x2000;
}
