#include "Aimbot.h"
#include "../core/Globals.h"
#include "../core/Offsets.h"
#include "../core/LocalPlayer.h"
#include "GameRuntime.h"

#include <windows.h>
#include <string.h>

static const struct EspTargetState* g_aimLocked;
static int g_aimKeyWasDown;
static struct Vec3 g_aimBonePos;
static int g_aimBoneValid;

void AimClearLock(void)
{
    g_aimLocked = 0;
    g_aimKeyWasDown = 0;
    g_aimBoneValid = 0;
    g_runtime.aimActive = 0;
    g_runtime.aimTargetValid = 0;
    g_runtime.aimOnTarget = 0;
    g_runtime.aimDelta.x = 0.0f;
    g_runtime.aimDelta.y = 0.0f;
}

int AimInFov(const struct Vec2* p, float radius)
{
    float cx, cy, dx, dy;
    if (!p)
        return 0;
    cx = (float)g_espWindowWidth * 0.5f;
    cy = (float)g_espWindowHeight * 0.5f;
    dx = p->x - cx;
    dy = p->y - cy;
    return (dx * dx + dy * dy) <= radius * radius;
}

static int AimReadTargetPos(const struct EspTargetState* t, int boneIndex, struct Vec3* out)
{
    qword actor;
    qword nested = 0;
    struct Vec3 base;
    float boneOffZ;

    if (!out || !t)
        return 0;
    memset(out, 0, sizeof(*out));
    actor = t->actorAddr;
    if (!IsValidPtr(actor))
        return 0;
    if (GameReadActorPosition(actor, &base)) {
        boneOffZ = 90.0f;
        if (boneIndex == 1)
            boneOffZ = 70.0f;
        else if (boneIndex == 2)
            boneOffZ = 55.0f;
        else if (boneIndex == 3)
            boneOffZ = 40.0f;
        out->x = base.x;
        out->y = base.y;
        out->z = base.z + boneOffZ;
        g_aimBoneValid = 1;
        return 1;
    }
    if (GameSafeRead(actor + Off_ActorNested, &nested, 8) && IsValidPtr(nested)) {
        if (GameReadActorPosition(nested, &base)) {
            out->x = base.x;
            out->y = base.y;
            out->z = base.z + 70.0f;
            g_aimBoneValid = 1;
            return 1;
        }
    }
    if (t->world.x != 0.0f || t->world.y != 0.0f || t->world.z != 0.0f) {
        *out = t->world;
        out->z += 80.0f;
        g_aimBoneValid = 1;
        return 1;
    }
    return 0;
}

static const struct EspTargetState* AimPickTarget(const struct AimConfig* aim)
{
    int i;
    const struct EspTargetState* best = 0;
    float bestDist = 1.0e30f;
    float cx = (float)g_espWindowWidth * 0.5f;
    float cy = (float)g_espWindowHeight * 0.5f;
    if (!aim || !aim->enabled)
        return 0;
    for (i = 0; i < g_runtime.targetCount; i++) {
        const struct EspTargetState* t = &g_runtime.targets[i];
        float dx, d;
        if (!t->visible)
            continue;
        if (t->entityType == EntGenerator || t->entityType == EntHatch || t->entityType == EntTotem ||
            t->entityType == EntPallet || t->entityType == EntChest || t->entityType == EntExit ||
            t->entityType == EntTrap)
            continue;
        if (g_config.esp.maxDistance > 0.0f && t->distance > g_config.esp.maxDistance)
            continue;
        if (!AimInFov(&t->screenLerp, aim->fovRadius))
            continue;
        dx = t->screenLerp.x - cx;
        d = dx * dx + (t->screenLerp.y - cy) * (t->screenLerp.y - cy);
        if (d < bestDist) {
            bestDist = d;
            best = t;
        }
    }
    return best;
}

static float AimApproxAtan2(float y, float x)
{
    float absX = x < 0.0f ? -x : x;
    float absY = y < 0.0f ? -y : y;
    float a = absY / (absX + absY);
    float r = a * 0.97239411f + 0.21460183f;
    r = 1.57079633f - (absX + absY - a * absX - a * absY) * r;
    if (x < 0.0f)
        r = 3.14159265f - r;
    if (y < 0.0f)
        r = -r;
    return r;
}

static void AimWriteDelta(const struct AimConfig* cfg, const struct EspTargetState* t)
{
    float cx, cy, k;
    struct Vec3 bone;
    struct Vec3 lp;
    float dx, dy, dz;
    float distSq, dist;
    float targetYaw, targetPitch;
    float deltaYaw, deltaPitch;
    float smooth;

    if (!cfg || !t)
        return;
    cx = (float)g_espWindowWidth * 0.5f;
    cy = (float)g_espWindowHeight * 0.5f;
    g_aimBoneValid = 0;
    memset(&g_aimBonePos, 0, sizeof(g_aimBonePos));
    if (!AimReadTargetPos(t, cfg->bonePriority[0], &bone)) {
        g_runtime.aimActive = 0;
        g_runtime.aimTargetValid = 0;
        return;
    }
    g_aimBonePos = bone;
    memset(&lp, 0, sizeof(lp));
    if (!LocalPlayerReadPosition(&lp)) {
        g_runtime.aimActive = 0;
        g_runtime.aimTargetValid = 0;
        return;
    }
    dx = bone.x - lp.x;
    dy = bone.y - lp.y;
    dz = bone.z - lp.z;
    distSq = dx * dx + dy * dy;
    if (distSq < 1.0f)
        distSq = 1.0f;
    dist = distSq;
    {
        int n;
        for (n = 0; n < 6; n++)
            dist = 0.5f * (dist + distSq / dist);
    }
    targetYaw = AimApproxAtan2(dy, dx) * 57.2957795f;
    targetPitch = AimApproxAtan2(dz, dist) * 57.2957795f;
    deltaYaw = targetYaw;
    deltaPitch = targetPitch;
    smooth = cfg->smoothing;
    if (smooth < 1.0f)
        smooth = 1.0f;
    if (smooth > 50.0f)
        smooth = 50.0f;
    k = 1.0f / smooth;
    g_runtime.aimActive = 1;
    g_runtime.aimTargetValid = 1;
    g_runtime.aimTargetWorld = bone;
    g_runtime.aimDelta.x = deltaPitch * k;
    g_runtime.aimDelta.y = deltaYaw * k;
    if (t->visible) {
        float sx = t->screenLerp.x - cx;
        float sy = t->screenLerp.y - cy;
        g_runtime.aimOnTarget = (sx * sx + sy * sy) < 6.0f * 6.0f;
    } else {
        g_runtime.aimOnTarget = 0;
    }
}

void AimLogicApply(const struct AimConfig* cfg)
{
    const struct EspTargetState* t;

    if (!cfg || !cfg->enabled) {
        AimClearLock();
        return;
    }
    if (cfg->key != 0) {
        int down = (GetAsyncKeyState(cfg->key) & 0x8000) != 0;
        if (cfg->keyMode == AimKeyToggle) {
            if (down && !g_aimKeyWasDown)
                g_config.aim.enabled = g_config.aim.enabled ? 0 : 1;
            g_aimKeyWasDown = down;
            if (!g_config.aim.enabled) {
                AimClearLock();
                return;
            }
        } else if (!down) {
            AimClearLock();
            return;
        }
    }
    if (cfg->smoothing < 1.0f)
        return;
    __try {
        if (g_aimLocked && g_aimLocked->visible && AimInFov(&g_aimLocked->screenLerp, cfg->fovRadius) &&
            g_aimLocked->actorAddr)
            t = g_aimLocked;
        else {
            t = AimPickTarget(cfg);
            g_aimLocked = t;
        }
        if (!t) {
            AimClearLock();
            return;
        }
        AimWriteDelta(cfg, t);
    } __except (1) {
        AimClearLock();
    }
}
