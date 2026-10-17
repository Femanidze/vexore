#ifndef VEXORE_MENUSTYLES_H
#define VEXORE_MENUSTYLES_H

#include "../core/Types.h"

enum MenuTabId {
    MenuTabSidebar = 0,
    MenuTabEsp,
    MenuTabAuras,
    MenuTabAimbot,
    MenuTabSkillCheck,
    MenuTabCrosshair,
    MenuTabHitsound,
    MenuTabBlindBar,
    MenuTabFovSpeed,
    MenuTabMoonwalk,
    MenuTabConfigs,
    MenuTabWatermarks,
    MenuTabStreamproof,
    MenuTabCount
};

struct MenuStyleConst {
    float itemSpacing;
    float framePadding;
    float windowRounding;
    float grabMinSize;
    float tabWidth;
    float popupWidth;
    float sectionGap;
    float labelPad;
    float maxDistDefault;
    float smoothingDefault;
    float aimFovDefault;
    float aimSmoothDefault;
    float themePickSize;
    float opacityDefault;
    float aimBump;
    float moonwalkBump;
};

extern const struct MenuStyleConst g_menuStyle;
extern const char* const g_menuTabNames[MenuTabCount];
extern const char* const g_menuPopIds[];

void MenuStyleApplyTheme(struct MenuTheme* theme, float r, float g, float b, float alpha);
void MenuStyleClampTheme(struct MenuTheme* theme);
void MenuStyleResetDefault(struct MenuTheme* theme);
float MenuStyleOpacity(void);

#endif
