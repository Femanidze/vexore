#include "MenuRender.h"
#include "MenuStyles.h"
#include "MenuTabs.h"
#include "../core/Globals.h"

void MenuApplyTheme(const struct MenuTheme* theme)
{
    if (!theme)
        return;
    g_config.theme.r = theme->r;
    g_config.theme.g = theme->g;
    g_config.theme.b = theme->b;
    g_config.theme.alpha = theme->alpha;
}

void MenuRenderEspTab(struct EspConfig* esp)
{
    MenuTabsEsp(esp);
}

void MenuRenderAimTab(struct AimConfig* aim)
{
    struct CheatConfig tmp;
    if (!aim)
        return;
    tmp = g_config;
    MenuTabsAimbot(aim, &tmp);
}

void MenuRenderAuraTab(struct AuraConfig* aura)
{
    MenuTabsAuras(aura);
}

void MenuRenderSkillCheckTab(struct SkillCheckConfig* sc)
{
    MenuTabsSkillCheck(sc);
}

void MenuRenderSpeedTab(struct SpeedHackConfig* sh)
{
    struct CheatConfig tmp;
    if (!sh)
        return;
    tmp = g_config;
    tmp.speedHack = *sh;
    MenuTabsFovSpeed(&tmp);
    *sh = tmp.speedHack;
}

void MenuRenderMiscTab(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    MenuStyleClampTheme(&cfg->theme);
    MenuTabsDispatch(cfg, MenuTabSidebar);
}

void MenuRenderFrame(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    MenuApplyTheme(&cfg->theme);
    MenuStyleClampTheme(&cfg->theme);
    MenuRenderEspTab(&cfg->esp);
    MenuRenderAimTab(&cfg->aim);
    MenuRenderAuraTab(&cfg->aura);
    MenuRenderSkillCheckTab(&cfg->skillCheck);
    MenuRenderSpeedTab(&cfg->speedHack);
    MenuTabsCrosshair(cfg);
    MenuTabsHitsound(cfg);
    MenuTabsWatermarks(cfg);
    MenuTabsMoonwalk(cfg);
    MenuRenderMiscTab(cfg);
}
