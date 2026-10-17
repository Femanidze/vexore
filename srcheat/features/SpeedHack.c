#include "SpeedHack.h"
#include "../core/Globals.h"
#include "../core/Offsets.h"
#include "../core/LocalPlayer.h"
#include "GameRuntime.h"

#include <windows.h>
#include <string.h>

static struct {
    qword addr;
    float original;
    int tracked;
} s_speedTracked;

static int s_speedWanted;
static qword s_speedActor;

float SpeedHackClampValue(float v)
{
    if (v < 0.1f)
        return 1.0f;
    if (v > 10.0f)
        return 1.0f;
    return v;
}

float SpeedHackScaleDelta(float delta, const struct SpeedHackConfig* cfg)
{
    if (!cfg || !cfg->enabled)
        return delta;
    return delta * cfg->value;
}

static void SpeedTrackFloat(qword addr, float val)
{
    float orig = 0.0f;

    if (!IsValidPtr(addr))
        return;
    if (!s_speedTracked.tracked || s_speedTracked.addr != addr) {
        if (GameSafeRead(addr, &orig, 4)) {
            s_speedTracked.addr = addr;
            s_speedTracked.original = orig;
            s_speedTracked.tracked = 1;
        }
    }
    GameSafeWrite(addr, &val, 4);
}

static void SpeedRestoreTracked(void)
{
    if (!s_speedTracked.tracked)
        return;
    if (IsValidPtr(s_speedTracked.addr))
        GameSafeWrite(s_speedTracked.addr, &s_speedTracked.original, 4);
    s_speedTracked.tracked = 0;
    s_speedTracked.addr = 0;
    s_speedTracked.original = 0.0f;
}

static int SpeedProbe(qword addr, float value)
{
    float orig = 0.0f;
    float scaled;

    if (!IsValidPtr(addr))
        return 0;
    if (!GameSafeRead(addr, &orig, 4))
        return 0;
    if (orig != orig)
        return 0;
    if (orig < 50.0f || orig > 2000.0f)
        return 0;
    scaled = orig * value;
    if (scaled > 2000.0f)
        scaled = 2000.0f;
    if (scaled < 1.0f)
        return 0;
    SpeedTrackFloat(addr, scaled);
    return 1;
}

void SpeedHackApply(const struct SpeedHackConfig* cfg)
{
    qword actor;
    float value;
    float probe = 0.0f;
    int hit = 0;

    if (!cfg || !cfg->enabled) {
        if (s_speedWanted)
            SpeedRestoreTracked();
        s_speedWanted = 0;
        s_speedActor = 0;
        return;
    }
    value = cfg->value;
    if (value < 0.1f || value > 10.0f)
        value = SpeedHackClampValue(value);
    if (value == 1.0f) {
        if (s_speedWanted)
            SpeedRestoreTracked();
        s_speedWanted = 0;
        return;
    }
    __try {
        if (!g_connected || !g_gameBase)
            return;
        LocalPlayerResolve();
        actor = LocalPlayerActor();
        if (!IsValidPtr(actor))
            return;
        if (s_speedTracked.tracked && IsValidPtr(s_speedTracked.addr)) {
            float cur = s_speedTracked.original * value;
            if (cur > 2000.0f)
                cur = 2000.0f;
            if (cur < 1.0f)
                cur = 1.0f;
            GameSafeWrite(s_speedTracked.addr, &cur, 4);
            s_speedWanted = 1;
            s_speedActor = actor;
            return;
        }
        if (GameSafeRead(actor + Off_ActorProgress, &probe, 4) && probe >= 50.0f && probe <= 2000.0f) {
            if (SpeedProbe(actor + Off_ActorProgress, value))
                hit = 1;
        }
        if (!hit && GameSafeRead(actor + Off_ActorRecovery, &probe, 4) && probe >= 50.0f && probe <= 2000.0f) {
            if (SpeedProbe(actor + Off_ActorRecovery, value))
                hit = 1;
        }
        if (!hit) {
            qword comp = 0;
            if (GameSafeRead(actor + Off_ComponentOwner, &comp, 8) && IsValidPtr(comp)) {
                if (SpeedProbe(comp + Off_ComponentPosition, value))
                    hit = 1;
            }
        }
        if (hit) {
            s_speedWanted = 1;
            s_speedActor = actor;
        }
    } __except (1) {
    }
}

void SpeedHackRestore(void)
{
    SpeedRestoreTracked();
    s_speedWanted = 0;
    s_speedActor = 0;
}
