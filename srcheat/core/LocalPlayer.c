#include "LocalPlayer.h"
#include "Globals.h"
#include "Offsets.h"
#include "../features/GameRuntime.h"

#include <string.h>

struct LocalPlayerState g_localPlayer;

int LocalPlayerResolve(void)
{
    qword world;
    qword mid;
    qword list;
    unsigned int count = 0;
    unsigned int i;

    memset(&g_localPlayer, 0, sizeof(g_localPlayer));
    if (!g_gameBase || !g_connected)
        return 0;
    world = g_signatureOffA ? g_signatureOffA : g_gameBase;
    if (!GameSafeRead(world + Off_WorldToMid, &mid, 8))
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
    g_localPlayer.world = world;
    g_localPlayer.mid = mid;
    g_localPlayer.entityList = list;
    for (i = 0; i < count; i++) {
        qword entry = 0;
        qword actor = 0;
        unsigned int type = 0;
        if (!GameSafeRead(list + (qword)i * 8, &entry, 8))
            continue;
        if (!IsValidPtr(entry))
            continue;
        if (!GameSafeRead(entry + Off_EntityType, &type, 4))
            continue;
        if (type != Off_EntityTypeSurvivor)
            continue;
        if (!GameSafeRead(entry + Off_EntityActor, &actor, 8))
            continue;
        if (!IsValidPtr(actor))
            continue;
        g_localPlayer.localActor = actor;
        break;
    }
    g_localPlayer.valid = g_localPlayer.localActor ? 1 : 0;
    if (g_localPlayer.valid)
        g_localPlayer.healthPct = LocalPlayerReadHealthPct();
    return g_localPlayer.valid;
}

int LocalPlayerReadPosition(struct Vec3* out)
{
    qword actor;
    qword component = 0;
    struct Vec3 pos;

    if (!out)
        return 0;
    memset(out, 0, sizeof(*out));
    actor = g_localPlayer.localActor;
    if (!actor) {
        if (!LocalPlayerResolve())
            return 0;
        actor = g_localPlayer.localActor;
    }
    if (!IsValidPtr(actor))
        return 0;
    if (!GameSafeRead(actor + Off_ComponentOwner, &component, 8))
        return 0;
    if (!IsValidPtr(component))
        return 0;
    if (!GameSafeRead(component + Off_ComponentPosition, &pos, Off_ComponentPositionSize))
        return 0;
    if (pos.x != pos.x || pos.y != pos.y || pos.z != pos.z)
        return 0;
    *out = pos;
    g_localPlayer.position = pos;
    return 1;
}

float LocalPlayerReadHealthPct(void)
{
    float pct = 0.0f;

    if (!GameSafeRead(GlobalLocalHealthPct, &pct, 4))
        return 0.0f;
    if (pct != pct)
        return 0.0f;
    if (pct < 0.0f)
        return 0.0f;
    if (pct > ProgressMax)
        return ProgressMax;
    return pct;
}

qword LocalPlayerActor(void)
{
    if (!g_localPlayer.localActor)
        LocalPlayerResolve();
    return g_localPlayer.localActor;
}
