#ifndef VEXORE_MENUTABS_H
#define VEXORE_MENUTABS_H

#include "../core/Types.h"
#include "MenuStyles.h"

void MenuTabsEsp(struct EspConfig* esp);
void MenuTabsAuras(struct AuraConfig* aura);
void MenuTabsAimbot(struct AimConfig* aim, struct CheatConfig* cfg);
void MenuTabsSkillCheck(struct SkillCheckConfig* sc);
void MenuTabsCrosshair(struct CheatConfig* cfg);
void MenuTabsHitsound(struct CheatConfig* cfg);
void MenuTabsFovSpeed(struct CheatConfig* cfg);
void MenuTabsMoonwalk(struct CheatConfig* cfg);
void MenuTabsConfigs(struct CheatConfig* cfg);
void MenuTabsWatermarks(struct CheatConfig* cfg);
void MenuTabsStreamproof(struct CheatConfig* cfg);
void MenuTabsSidebar(struct CheatConfig* cfg, int activeTab);
void MenuTabsDispatch(struct CheatConfig* cfg, int activeTab);

#endif
