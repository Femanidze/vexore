#include "Esp.h"
#include "GameRuntime.h"
#include "../core/Globals.h"
#include "../core/Offsets.h"
#include "../core/LocalPlayer.h"

#include <string.h>

int EspProjectTarget(const struct Vec3* world, struct Vec2* out)
{
    struct Vec3 lp;
    float dx, dy, dz;
    float x, y, z;
    float fovScale;
    float w;

    if (!world || !out)
        return 0;
    if (g_espWindowWidth == 0 || g_espWindowHeight == 0)
        return 0;
    memset(&lp, 0, sizeof(lp));
    if (!LocalPlayerReadPosition(&lp)) {
        lp.x = 0.0f;
        lp.y = 0.0f;
        lp.z = 0.0f;
    }
    dx = world->x - lp.x;
    dy = world->y - lp.y;
    dz = world->z - lp.z;
    x = -dy;
    y = dz;
    z = dx;
    if (z <= 50.0f)
        return 0;
    fovScale = 900.0f;
    w = z;
    out->x = (x / w) * fovScale * (float)g_espWindowWidth * 0.5f + (float)g_espWindowWidth * 0.5f;
    out->y = -(y / w) * fovScale * (float)g_espWindowHeight * 0.5f + (float)g_espWindowHeight * 0.5f;
    if (out->x < -400.0f || out->x > (float)g_espWindowWidth + 400.0f)
        return 0;
    if (out->y < -400.0f || out->y > (float)g_espWindowHeight + 400.0f)
        return 0;
    return 1;
}

float EspComputeFade(const struct EspConfig* cfg, float distance)
{
    float fade;
    if (!cfg)
        return 1.0f;
    if (cfg->boxDistanceFade <= 0.0f)
        return 1.0f;
    if (distance <= cfg->boxDistanceFade)
        return 1.0f;
    fade = 1.0f - (distance - cfg->boxDistanceFade) / (cfg->boxDistanceFade + 1.0f);
    if (fade < 0.0f)
        fade = 0.0f;
    return fade;
}

void EspUppercaseName(char* name)
{
    char* p;
    if (!name)
        return;
    p = name;
    while (*p) {
        if (*p >= 'a' && *p <= 'z')
            *p = (char)(*p - 'a' + 'A');
        p++;
    }
}

void EspResolveNames(struct EspConfig* cfg)
{
    int i;
    if (!cfg || !cfg->enabled)
        return;
    if (!cfg->nameUppercase)
        return;
    for (i = 0; i < g_runtime.targetCount; i++)
        EspUppercaseName(g_runtime.targets[i].name);
}

void EspOverlayApply(const struct EspConfig* cfg)
{
    int i;
    if (!cfg || !cfg->enabled)
        return;
    for (i = 0; i < g_runtime.targetCount; i++) {
        struct EspTargetState* t = &g_runtime.targets[i];
        if (cfg->maxDistance > 0.0f && t->distance > cfg->maxDistance) {
            t->visible = 0;
            continue;
        }
        t->visible = EspProjectTarget(&t->world, &t->screen);
        if (t->visible) {
            float k = cfg->lerpSmooth;
            if (k <= 0.0f)
                k = 0.15f;
            if (k > 1.0f)
                k = 1.0f;
            if (g_runtime.featureFrame <= 1) {
                t->screenLerp.x = t->screen.x;
                t->screenLerp.y = t->screen.y;
            } else {
                t->screenLerp.x = t->screenLerp.x + (t->screen.x - t->screenLerp.x) * k;
                t->screenLerp.y = t->screenLerp.y + (t->screen.y - t->screenLerp.y) * k;
            }
        }
    }
    EspResolveNames(cfg);
}
