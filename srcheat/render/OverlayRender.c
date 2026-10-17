#include "OverlayRender.h"
#include "../core/Globals.h"
#include "../features/GameRuntime.h"
#include "../features/Esp.h"
#include "../features/Aura.h"
#include "../menu/MenuRender.h"

#include <stdio.h>
#include <string.h>

struct OverlayDrawCmd {
    int type;
    float x0, y0, x1, y1;
    float thickness;
    struct ColorRgba color;
    char text[64];
    float textSize;
};

#define OVERLAY_MAX_CMDS 512

static struct OverlayDrawCmd g_cmds[OVERLAY_MAX_CMDS];
static int g_cmdCount;

static void PushCmd(struct OverlayDrawCmd* c)
{
    if (g_cmdCount >= OVERLAY_MAX_CMDS || !c)
        return;
    g_cmds[g_cmdCount++] = *c;
}

static struct ColorRgba PickHealthColor(const struct EspConfig* cfg, int healthState)
{
    if (healthState == 3)
        return cfg->colBoxDowned;
    if (healthState == 2)
        return cfg->colBoxInjured;
    if (healthState == 1)
        return cfg->colBoxHealthy;
    return cfg->colBoxSurv;
}

static struct ColorRgba PickRoleColor(const struct EspConfig* cfg, const struct EspTargetState* t)
{
    if (t->entityType == 1)
        return cfg->colBoxSurv;
    if (t->entityType == 2)
        return cfg->colBoxKiller;
    return PickHealthColor(cfg, t->healthState);
}

static float EspBoxHeight(const struct EspTargetState* t)
{
    float h = 40.0f + t->distance * 0.05f;
    if (h < 20.0f)
        h = 20.0f;
    if (h > 200.0f)
        h = 200.0f;
    return h;
}

static void PushBoxCmd(const struct EspConfig* cfg, const struct EspTargetState* t, float h, float fade, int filled)
{
    struct OverlayDrawCmd c;
    struct ColorRgba col;
    float hw = 14.0f;
    col = PickRoleColor(cfg, t);
    col.a *= fade;
    if (cfg->boxType == BoxEspCorner)
        hw = 10.0f;
    if (cfg->boxType == BoxEsp3D)
        hw = 16.0f;
    memset(&c, 0, sizeof(c));
    c.type = filled ? 2 : 1;
    if (cfg->boxType == BoxEspCorner || cfg->boxType == BoxEsp3D)
        c.type = 7;
    c.x0 = t->screenLerp.x - hw;
    c.y0 = t->screenLerp.y - h;
    c.x1 = t->screenLerp.x + hw;
    c.y1 = t->screenLerp.y;
    c.thickness = cfg->boxThickness;
    c.color = col;
    PushCmd(&c);
    if (cfg->boxOutline) {
        struct OverlayDrawCmd o = c;
        o.thickness = cfg->boxThickness + 1.0f;
        o.color.a = col.a * 0.4f;
        o.x0 -= 1.0f;
        o.y0 -= 1.0f;
        o.x1 += 1.0f;
        o.y1 += 1.0f;
        PushCmd(&o);
    }
}

static void PushHealthBar(const struct EspConfig* cfg, const struct EspTargetState* t, float h, float fade)
{
    struct OverlayDrawCmd c;
    float bx = t->screenLerp.x - 20.0f;
    float fill;
    memset(&c, 0, sizeof(c));
    c.type = 3;
    c.x0 = bx;
    c.y0 = t->screenLerp.y - h;
    c.x1 = bx + 3.0f;
    c.y1 = t->screenLerp.y;
    c.color = cfg->colHealthBar;
    c.color.a *= fade;
    PushCmd(&c);
    fill = t->progressPct;
    if (fill < 0.0f)
        fill = 0.0f;
    if (fill > 100.0f)
        fill = 100.0f;
    fill = fill * 0.01f;
    c.y0 = t->screenLerp.y - h * fill;
    c.color = PickHealthColor(cfg, t->healthState);
    c.color.a *= fade;
    PushCmd(&c);
}

static void PushNameCmd(const struct EspConfig* cfg, const struct EspTargetState* t, float h, float fade)
{
    struct OverlayDrawCmd c;
    memset(&c, 0, sizeof(c));
    c.type = 4;
    c.x0 = t->screenLerp.x + cfg->nicknameOffX;
    c.y0 = t->screenLerp.y + cfg->nicknameOffY - h;
    c.color = cfg->colName;
    if (cfg->nameShadow) {
        struct OverlayDrawCmd s = c;
        s.x0 += 1.0f;
        s.y0 += 1.0f;
        s.color.r = 0.0f;
        s.color.g = 0.0f;
        s.color.b = 0.0f;
        s.color.a = fade;
        strncpy(s.text, t->name, sizeof(s.text) - 1);
        s.textSize = cfg->nameScale;
        PushCmd(&s);
    }
    c.color.a *= fade;
    c.textSize = cfg->nameScale;
    strncpy(c.text, t->name, sizeof(c.text) - 1);
    PushCmd(&c);
}

static void PushSkeletonCmd(const struct EspConfig* cfg, const struct EspTargetState* t, float h, float fade)
{
    struct OverlayDrawCmd c;
    float mid;
    if (cfg->skeletonType == SkeletonDots) {
        memset(&c, 0, sizeof(c));
        c.type = 8;
        c.x0 = t->screenLerp.x;
        c.y0 = t->screenLerp.y - h;
        c.color = PickRoleColor(cfg, t);
        c.color.a *= fade;
        PushCmd(&c);
        c.y0 = t->screenLerp.y - h * 0.5f;
        PushCmd(&c);
        c.y0 = t->screenLerp.y;
        PushCmd(&c);
        return;
    }
    mid = t->screenLerp.y - h * 0.5f;
    memset(&c, 0, sizeof(c));
    c.type = 5;
    c.x0 = t->screenLerp.x;
    c.y0 = t->screenLerp.y - h;
    c.x1 = t->screenLerp.x;
    c.y1 = t->screenLerp.y;
    c.thickness = 1.0f;
    c.color = PickRoleColor(cfg, t);
    c.color.a *= fade * 0.7f;
    PushCmd(&c);
    c.x0 = t->screenLerp.x - 8.0f;
    c.y0 = mid;
    c.x1 = t->screenLerp.x + 8.0f;
    c.y1 = mid;
    PushCmd(&c);
    if (cfg->skeletonType == SkeletonBoth) {
        c.x0 = t->screenLerp.x - 6.0f;
        c.y0 = t->screenLerp.y;
        c.x1 = t->screenLerp.x;
        c.y1 = mid;
        PushCmd(&c);
        c.x0 = t->screenLerp.x + 6.0f;
        PushCmd(&c);
    }
}

void OverlayBeginFrame(void)
{
    g_cmdCount = 0;
}

void OverlayDrawEsp(const struct EspConfig* cfg)
{
    int i;
    float fade;
    if (!cfg || !cfg->enabled)
        return;
    for (i = 0; i < g_runtime.targetCount; i++) {
        const struct EspTargetState* t = &g_runtime.targets[i];
        float h;
        if (!t->visible)
            continue;
        if (cfg->maxDistance > 0.0f && t->distance > cfg->maxDistance)
            continue;
        fade = EspComputeFade(cfg, t->distance);
        h = EspBoxHeight(t);
        PushBoxCmd(cfg, t, h, fade, cfg->boxFill);
        if (cfg->showHealthBar)
            PushHealthBar(cfg, t, h, fade);
        if (cfg->nameScale > 0.0f && t->name[0])
            PushNameCmd(cfg, t, h, fade);
        if (cfg->skeletonType != SkeletonDots && t->healthState >= 0)
            PushSkeletonCmd(cfg, t, h, fade);
        else if (cfg->skeletonType == SkeletonDots)
            PushSkeletonCmd(cfg, t, h, fade);
    }
}

void OverlayDrawAura(const struct AuraConfig* cfg)
{
    int i;
    if (!cfg || !cfg->masterSwitch)
        return;
    for (i = 0; i < g_runtime.targetCount; i++) {
        const struct EspTargetState* t = &g_runtime.targets[i];
        struct OverlayDrawCmd c;
        const struct AuraToggle* use;
        if (t->distance <= 0.0f)
            continue;
        use = AuraSelectToggle(cfg, t);
        if (!use && !(cfg->trapsMaster && t->distance <= cfg->trapsDistance))
            continue;
        memset(&c, 0, sizeof(c));
        c.type = 6;
        c.x0 = t->screenLerp.x - 8.0f;
        c.y0 = t->screenLerp.y - 8.0f;
        c.x1 = t->screenLerp.x + 8.0f;
        c.y1 = t->screenLerp.y + 8.0f;
        c.color = use ? use->color : cfg->colorTrapBear;
        PushCmd(&c);
    }
}

void OverlayDrawCrosshair(const struct CheatConfig* cfg)
{
    struct OverlayDrawCmd c;
    float cx, cy;
    if (!cfg)
        return;
    cx = (float)g_espWindowWidth * 0.5f;
    cy = (float)g_espWindowHeight * 0.5f;
    memset(&c, 0, sizeof(c));
    c.type = 1;
    c.x0 = cx - 8.0f;
    c.y0 = cy;
    c.x1 = cx + 8.0f;
    c.y1 = cy;
    c.thickness = 1.5f;
    c.color.r = 1.0f;
    c.color.g = 1.0f;
    c.color.b = 1.0f;
    c.color.a = 0.9f;
    PushCmd(&c);
    c.x0 = cx;
    c.y0 = cy - 8.0f;
    c.x1 = cx;
    c.y1 = cy + 8.0f;
    PushCmd(&c);
}

void OverlayDrawWatermark(const struct CheatConfig* cfg)
{
    struct OverlayDrawCmd c;
    if (!cfg)
        return;
    memset(&c, 0, sizeof(c));
    c.type = 4;
    c.x0 = 12.0f;
    c.y0 = 12.0f;
    c.textSize = 16.0f;
    c.color = cfg->esp.colName;
    strncpy(c.text, "VEXORE", sizeof(c.text) - 1);
    PushCmd(&c);
}

void OverlayEndFrame(void)
{
    if (g_cmdCount > OVERLAY_MAX_CMDS)
        g_cmdCount = OVERLAY_MAX_CMDS;
}

void OverlayRenderAll(const struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    OverlayBeginFrame();
    OverlayDrawEsp(&cfg->esp);
    OverlayDrawAura(&cfg->aura);
    if (cfg->theme.alpha > 0.0f)
        OverlayDrawCrosshair(cfg);
    OverlayDrawWatermark(cfg);
    MenuRenderFrame((struct CheatConfig*)cfg);
    OverlayEndFrame();
}
