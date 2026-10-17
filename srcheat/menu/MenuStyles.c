#include "MenuStyles.h"

const struct MenuStyleConst g_menuStyle = {
    8.0f,
    4.0f,
    6.0f,
    10.0f,
    110.0f,
    280.0f,
    18.0f,
    12.0f,
    155.0f,
    180.0f,
    250.0f,
    6.0f,
    4.0f,
    0.92f,
    25.0f,
    155.0f
};

const char* const g_menuTabNames[MenuTabCount] = {
    "Sidebar",
    "Enable ESP",
    "AURAS",
    "Enable Aimbot",
    "Enable SkillCheck",
    "Enable Crosshair",
    "Enable Hitsound",
    "Blinding Progress",
    "FOV Changer",
    "Auto Moonwalk",
    "Configs",
    "Watermarks",
    "Streamproof"
};

const char* const g_menuPopIds[] = {
    "main_esp_pop",
    "names_pop",
    "boxes_pop",
    "status_pop",
    "anticamp_pop",
    "skele_pop",
    "snap_pop",
    "team_check_pop",
    "gens_pop",
    "pallets_pop",
    "totems_pop",
    "exits_pop",
    "windows_pop",
    "hooks_pop",
    "chests_pop",
    "hatch_pop",
    "traps_pop",
    "aim_main_pop",
    "crosshair_pop_sub",
    "hitsound_pop_sub",
    "blind_bar_pop_sub",
    "skillcheck_pop_sub",
    "watermarks_popup",
    "menu_color_popup",
    "cfg_rows_zone",
    0
};

void MenuStyleApplyTheme(struct MenuTheme* theme, float r, float g, float b, float alpha)
{
    if (!theme)
        return;
    theme->r = r;
    theme->g = g;
    theme->b = b;
    theme->alpha = alpha;
}

void MenuStyleClampTheme(struct MenuTheme* theme)
{
    if (!theme)
        return;
    if (theme->r < 0.0f)
        theme->r = 0.0f;
    if (theme->r > 1.0f)
        theme->r = 1.0f;
    if (theme->g < 0.0f)
        theme->g = 0.0f;
    if (theme->g > 1.0f)
        theme->g = 1.0f;
    if (theme->b < 0.0f)
        theme->b = 0.0f;
    if (theme->b > 1.0f)
        theme->b = 1.0f;
    if (theme->alpha < 0.0f)
        theme->alpha = 0.0f;
    if (theme->alpha > 1.0f)
        theme->alpha = 1.0f;
}

void MenuStyleResetDefault(struct MenuTheme* theme)
{
    if (!theme)
        return;
    theme->r = 0.55f;
    theme->g = 0.35f;
    theme->b = 0.95f;
    theme->alpha = g_menuStyle.opacityDefault;
}

float MenuStyleOpacity(void)
{
    return g_menuStyle.opacityDefault;
}
