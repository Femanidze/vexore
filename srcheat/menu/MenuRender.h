#ifndef VEXORE_MENURENDER_H
#define VEXORE_MENURENDER_H

#include "../core/Types.h"

void MenuRenderFrame(struct CheatConfig* cfg);
void MenuRenderEspTab(struct EspConfig* esp);
void MenuRenderAimTab(struct AimConfig* aim);
void MenuRenderAuraTab(struct AuraConfig* aura);
void MenuRenderSkillCheckTab(struct SkillCheckConfig* sc);
void MenuRenderSpeedTab(struct SpeedHackConfig* sh);
void MenuRenderMiscTab(struct CheatConfig* cfg);
void MenuApplyTheme(const struct MenuTheme* theme);

#endif
