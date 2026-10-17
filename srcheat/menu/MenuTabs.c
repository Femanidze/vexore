#include "MenuTabs.h"
#include "MenuStyles.h"
#include "../core/Globals.h"
#include "../config/ConfigKeys.h"

#include <string.h>

struct MenuLabel {
    const char* text;
    const char* key;
    int slot;
};

enum MenuSlot {
    SlotNone = 0,
    SlotEspEnabled,
    SlotEspBoxType,
    SlotEspBoxOutline,
    SlotEspBoxFill,
    SlotEspBoxFillAlpha,
    SlotEspBoxGradient,
    SlotEspBoxThickness,
    SlotEspBoxDistFade,
    SlotEspShowHealth,
    SlotEspShowRecover,
    SlotEspMaxDist,
    SlotEspLerp,
    SlotEspNameUpper,
    SlotEspNameShadow,
    SlotEspNameScale,
    SlotEspSkeletonType,
    SlotEspSkeletonDots,
    SlotAimEnabled,
    SlotAimKey,
    SlotAimKeyMode,
    SlotAimFovRadius,
    SlotAimSmoothing,
    SlotAimDrawFov,
    SlotAimTargetLock,
    SlotAuraMaster,
    SlotAuraHatch,
    SlotAuraHooks,
    SlotAuraChests,
    SlotAuraTotems,
    SlotAuraPallets,
    SlotAuraTrapBear,
    SlotAuraTrapZombie,
    SlotAuraWindowMax,
    SlotAuraBrightness,
    SlotSkillEnabled,
    SlotSkillZone,
    SlotSkillGreat,
    SlotSpeedEnabled,
    SlotSpeedValue,
    SlotThemeR,
    SlotThemeG,
    SlotThemeB,
    SlotThemeAlpha,
    SlotCrosshairEnabled,
    SlotCrosshairType,
    SlotCrosshairSize,
    SlotCrosshairThickness,
    SlotCrosshairGap,
    SlotHitsoundEnabled,
    SlotHitsoundType,
    SlotHitsoundVolume,
    SlotWatermarkEnabled,
    SlotWatermarkLogo,
    SlotWatermarkTime,
    SlotWatermarkFps,
    SlotMoonwalkEnabled,
    SlotMoonwalkKey,
    SlotFovEnabled,
    SlotFovValue,
    SlotStreamproof,
    SlotRemoveBlackBars,
    SlotWiggleEnabled,
    SlotBlindBarEnabled,
    SlotBlindBarAutoHide,
    SlotBlindBarStyle,
    SlotBlindBarTextType,
    SlotKillerDetector,
    SlotKillerNotify,
    SlotKillerLang,
    SlotSnaplines,
    SlotAntiCampEnabled,
    SlotScratchMark
};

static float ClampF(float v, float lo, float hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

static int ClampI(int v, int lo, int hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

static int LabelCount(const struct MenuLabel* arr, int cap)
{
    int n = 0;
    while (n < cap && arr[n].text)
        n++;
    return n;
}

static void ApplyLabels(const struct MenuLabel* arr, int cap, struct CheatConfig* cfg)
{
    int i;
    int n = LabelCount(arr, cap);
    if (!cfg)
        return;
    for (i = 0; i < n; i++) {
        switch (arr[i].slot) {
        case SlotEspEnabled:
            break;
        case SlotEspBoxType:
            cfg->esp.boxType = ClampI(cfg->esp.boxType, BoxEsp2D, BoxEspOffscreen);
            break;
        case SlotEspBoxOutline:
            break;
        case SlotEspBoxFill:
            break;
        case SlotEspBoxFillAlpha:
            cfg->esp.boxFillAlpha = ClampF(cfg->esp.boxFillAlpha, 0.0f, 1.0f);
            break;
        case SlotEspBoxGradient:
            break;
        case SlotEspBoxThickness:
            cfg->esp.boxThickness = ClampF(cfg->esp.boxThickness, 0.5f, 8.0f);
            break;
        case SlotEspBoxDistFade:
            cfg->esp.boxDistanceFade = ClampF(cfg->esp.boxDistanceFade, 0.0f, 500.0f);
            break;
        case SlotEspShowHealth:
            break;
        case SlotEspShowRecover:
            break;
        case SlotEspMaxDist:
            cfg->esp.maxDistance = ClampF(cfg->esp.maxDistance, 0.0f, 1000.0f);
            break;
        case SlotEspLerp:
            cfg->esp.lerpSmooth = ClampF(cfg->esp.lerpSmooth, 0.01f, 1.0f);
            break;
        case SlotEspNameUpper:
            break;
        case SlotEspNameShadow:
            break;
        case SlotEspNameScale:
            cfg->esp.nameScale = ClampF(cfg->esp.nameScale, 8.0f, 32.0f);
            break;
        case SlotEspSkeletonType:
            cfg->esp.skeletonType = ClampI(cfg->esp.skeletonType, SkeletonLines, SkeletonBoth);
            break;
        case SlotEspSkeletonDots:
            break;
        case SlotAimEnabled:
            break;
        case SlotAimKey:
            cfg->aim.key = ClampI(cfg->aim.key, 0, 255);
            break;
        case SlotAimKeyMode:
            cfg->aim.keyMode = ClampI(cfg->aim.keyMode, AimKeyHold, AimKeyToggle);
            break;
        case SlotAimFovRadius:
            cfg->aim.fovRadius = ClampF(cfg->aim.fovRadius, 10.0f, 1000.0f);
            break;
        case SlotAimSmoothing:
            cfg->aim.smoothing = ClampF(cfg->aim.smoothing, 1.0f, 50.0f);
            break;
        case SlotAimDrawFov:
            break;
        case SlotAimTargetLock:
            break;
        case SlotAuraMaster:
            break;
        case SlotAuraHatch:
            
            cfg->aura.hatch.distance = ClampF(cfg->aura.hatch.distance, 0.0f, 200.0f);
            break;
        case SlotAuraHooks:
            
            cfg->aura.hooks.distance = ClampF(cfg->aura.hooks.distance, 0.0f, 200.0f);
            break;
        case SlotAuraChests:
            
            cfg->aura.chests.distance = ClampF(cfg->aura.chests.distance, 0.0f, 200.0f);
            break;
        case SlotAuraTotems:
            
            cfg->aura.totems.distance = ClampF(cfg->aura.totems.distance, 0.0f, 200.0f);
            break;
        case SlotAuraPallets:
            
            cfg->aura.pallets.distance = ClampF(cfg->aura.pallets.distance, 0.0f, 200.0f);
            break;
        case SlotAuraTrapBear:
            break;
        case SlotAuraTrapZombie:
            break;
        case SlotAuraWindowMax:
            cfg->aura.windowAuraMaxDist = ClampF(cfg->aura.windowAuraMaxDist, 0.0f, 200.0f);
            break;
        case SlotAuraBrightness:
            break;
        case SlotSkillEnabled:
            break;
        case SlotSkillZone:
            cfg->skillCheck.zoneMode = ClampI(cfg->skillCheck.zoneMode, SkillCheckDefaultZones, SkillCheckWideGreat);
            break;
        case SlotSkillGreat:
            cfg->skillCheck.greatChance = ClampF(cfg->skillCheck.greatChance, 0.0f, 100.0f);
            break;
        case SlotSpeedEnabled:
            break;
        case SlotSpeedValue:
            cfg->speedHack.value = ClampF(cfg->speedHack.value, 0.1f, 10.0f);
            break;
        case SlotThemeR:
            cfg->theme.r = ClampF(cfg->theme.r, 0.0f, 1.0f);
            break;
        case SlotThemeG:
            cfg->theme.g = ClampF(cfg->theme.g, 0.0f, 1.0f);
            break;
        case SlotThemeB:
            cfg->theme.b = ClampF(cfg->theme.b, 0.0f, 1.0f);
            break;
        case SlotThemeAlpha:
            cfg->theme.alpha = ClampF(cfg->theme.alpha, 0.0f, 1.0f);
            break;
        default:
            break;
        }
    }
}

static const struct MenuLabel g_espLabels[] = {
    { "Enable ESP", KEY_ESP_ENABLED, SlotEspEnabled },
    { "Max Distance", KEY_ESP_MAX_DIST, SlotEspMaxDist },
    { "ESP Smoothing", KEY_ESP_LERP, SlotEspLerp },
    { "Box Style", KEY_ESP_BOX_TYPE, SlotEspBoxType },
    { "Box Outline", KEY_ESP_BOX_OUTLINE, SlotEspBoxOutline },
    { "Color Gradient", KEY_ESP_BOX_GRADIENT, SlotEspBoxGradient },
    { "Distance Fading", KEY_ESP_BOX_DIST_FADE, SlotEspBoxDistFade },
    { "Filled Background", KEY_ESP_BOX_FILL, SlotEspBoxFill },
    { "Fill Alpha", KEY_ESP_BOX_FILL_ALPHA, SlotEspBoxFillAlpha },
    { "Show Health Bar", KEY_ESP_SHOW_HEALTH, SlotEspShowHealth },
    { "Line Thickness", KEY_ESP_BOX_THICKNESS, SlotEspBoxThickness },
    { "Text Shadow", KEY_ESP_NAME_SHADOW, SlotEspNameShadow },
    { "Uppercase", KEY_ESP_NAME_UPPER, SlotEspNameUpper },
    { "Font Scale", KEY_ESP_NAME_SCALE, SlotEspNameScale },
    { "Skeleton Style", KEY_ESP_SKELETON_TYPE, SlotEspSkeletonType },
    { "Show Joints (Dots)", KEY_ESP_SKELETON_DOTS, SlotEspSkeletonDots },
    { "Force Anim Tick (Fix Lag/Invis)", KEY_ESP_SKELETON_FIX, SlotNone },
    { "Show Recover Bar", KEY_ESP_SHOW_RECOVER, SlotEspShowRecover },
    { 0, 0, 0 }
};

static const struct MenuLabel g_aimLabels[] = {
    { "Enable Aimbot", KEY_AIM_ENABLED, SlotAimEnabled },
    { "Target Selection", KEY_AIM_TARGET_SELECTION, SlotNone },
    { "Key Activation Mode", KEY_AIM_KEY_MODE, SlotAimKeyMode },
    { "Draw FOV Circle", KEY_AIM_DRAW_FOV, SlotAimDrawFov },
    { "FOV Radius", KEY_AIM_FOV_RADIUS, SlotAimFovRadius },
    { "Smoothing (Legit)", KEY_AIM_SMOOTHING, SlotAimSmoothing },
    { "Target Lock", KEY_AIM_TARGET_LOCK, SlotAimTargetLock },
    { "Assigned Keybind:", KEY_AIM_KEY, SlotAimKey },
    { "Scan Head", KEY_AIM_SELECTED_BONES0, SlotNone },
    { "Scan Neck", KEY_AIM_SELECTED_BONES1, SlotNone },
    { "Scan Chest", KEY_AIM_SELECTED_BONES2, SlotNone },
    { "Scan Pelvis", KEY_AIM_SELECTED_BONES3, SlotNone },
    { 0, 0, 0 }
};

static const struct MenuLabel g_auraLabels[] = {
    { "Master Aura Switch", KEY_AURA_MASTER, SlotAuraMaster },
    { "Enable Aura", KEY_AURA_HATCH_EN, SlotAuraHatch },
    { "Hatch", KEY_AURA_HATCH_DIST, SlotAuraHatch },
    { "Meat Hooks", KEY_AURA_HOOKS_EN, SlotAuraHooks },
    { "Chests", KEY_AURA_CHESTS_EN, SlotAuraChests },
    { "Totems", KEY_AURA_TOTEMS_EN, SlotAuraTotems },
    { "Pallets", KEY_AURA_PALLET_EN, SlotAuraPallets },
    { "Bear Traps (Trapper)", KEY_AURA_TRAP_BEAR, SlotAuraTrapBear },
    { "Zombies (Nemesis)", KEY_AURA_TRAP_ZOMBIE, SlotAuraTrapZombie },
    { "Global Max Dist", KEY_WINDOW_AURA_MAX, SlotAuraWindowMax },
    { "Aura Brightness", "aura_brightness", SlotAuraBrightness },
    { "Generators", "aura_generators_enabled", SlotNone },
    { "Exit Gates", "aura_exits_enabled", SlotNone },
    { "Filter Entities:", "aura_traps_master", SlotNone },
    { 0, 0, 0 }
};

static const struct MenuLabel g_skillLabels[] = {
    { "Enable SkillCheck", KEY_SKILLCHECK_ENABLED, SlotSkillEnabled },
    { "Target Zones", KEY_SKILLCHECK_ZONE, SlotSkillZone },
    { "Great Chance %", KEY_SKILLCHECK_GREAT, SlotSkillGreat },
    { "Only Great (White)", "skillcheck_only_great", SlotNone },
    { "Great + Good", "skillcheck_great_good", SlotNone },
    { 0, 0, 0 }
};

static const struct MenuLabel g_speedLabels[] = {
    { "SpeedHack", KEY_SPEEDHACK_ENABLED, SlotSpeedEnabled },
    { "Speed Value", KEY_SPEEDHACK_VALUE, SlotSpeedValue },
    { "FOV Changer", "FovEnabled", SlotFovEnabled },
    { "FOV Value", "RealFovValue", SlotFovValue },
    { 0, 0, 0 }
};

static const struct MenuLabel g_crosshairLabels[] = {
    { "Enable Crosshair", KEY_CROSSHAIR_ENABLED, SlotCrosshairEnabled },
    { "Cross", "crosshair_cross", SlotCrosshairType },
    { "Circle", "crosshair_circle", SlotCrosshairType },
    { "T-Style", "crosshair_tstyle", SlotCrosshairType },
    { "Thickness", KEY_CROSSHAIR_THICKNESS, SlotCrosshairThickness },
    { 0, 0, 0 }
};

static const struct MenuLabel g_hitsoundLabels[] = {
    { "Enable Hitsound", KEY_HITSOUND_ENABLED, SlotHitsoundEnabled },
    { "Plastic Hit", "hitsound_plastic", SlotHitsoundType },
    { "Ding Bell", "hitsound_bell", SlotHitsoundType },
    { "Metal Click", "hitsound_metal", SlotHitsoundType },
    { "Volume", KEY_HITSOUND_VOLUME, SlotHitsoundVolume },
    { 0, 0, 0 }
};

static const struct MenuLabel g_watermarkLabels[] = {
    { "Watermarks", KEY_WATERMARK_ENABLED, SlotWatermarkEnabled },
    { "Show VEXORE Logo", KEY_WATERMARK_LOGO, SlotWatermarkLogo },
    { "Show System Time", KEY_WATERMARK_TIME, SlotWatermarkTime },
    { "Show Real FPS", KEY_WATERMARK_FPS, SlotWatermarkFps },
    { 0, 0, 0 }
};

static const struct MenuLabel g_moonwalkLabels[] = {
    { "Auto Moonwalk", "MoonwalkEnabled", SlotMoonwalkEnabled },
    { "Moonwalk Key", "MoonwalkKey", SlotMoonwalkKey },
    { "Enable Auto-Wiggle", "WiggleEnabled", SlotWiggleEnabled },
    { "Remove Black Bars", "RemoveBlackBars", SlotRemoveBlackBars },
    { "Streamproof", "obs_clean", SlotStreamproof },
    { "Footstep Visibility", "footstep_visibility", SlotNone },
    { 0, 0, 0 }
};

static const struct MenuLabel g_miscLabels[] = {
    { "Menu Color", KEY_MENU_THEME_R, SlotThemeR },
    { "Opacity", KEY_MENU_THEME_ALPHA, SlotThemeAlpha },
    { "Blinding Progress", KEY_BLIND_BAR_ENABLED, SlotBlindBarEnabled },
    { "Auto-Hide (< 0.1%)", KEY_BLIND_BAR_AUTOHIDE, SlotBlindBarAutoHide },
    { "Position Style", KEY_BLIND_BAR_STYLE, SlotBlindBarStyle },
    { "Text Format", KEY_BLIND_BAR_TEXT_TYPE, SlotBlindBarTextType },
    { "Enable Killer Detector", "lobby_killer_detector", SlotKillerDetector },
    { "Show Screen Notifications", "lobby_killer_notifications", SlotKillerNotify },
    { "Killer Name Language", "lobby_killer_lang", SlotKillerLang },
    { "Show Snaplines", "show_snaplines", SlotSnaplines },
    { "Anti-Camp Settings", "ac_enabled", SlotAntiCampEnabled },
    { "Scratch Mark ESP", KEY_SCRATCH_MARK_ESP, SlotScratchMark },
    { 0, 0, 0 }
};

void MenuTabsEsp(struct EspConfig* esp)
{
    struct CheatConfig tmp;
    if (!esp)
        return;
    memset(&tmp, 0, sizeof(tmp));
    tmp.esp = *esp;
    ApplyLabels(g_espLabels, (int)(sizeof(g_espLabels) / sizeof(g_espLabels[0])), &tmp);
    *esp = tmp.esp;
}

void MenuTabsAuras(struct AuraConfig* aura)
{
    struct CheatConfig tmp;
    if (!aura)
        return;
    memset(&tmp, 0, sizeof(tmp));
    tmp.aura = *aura;
    ApplyLabels(g_auraLabels, (int)(sizeof(g_auraLabels) / sizeof(g_auraLabels[0])), &tmp);
    *aura = tmp.aura;
}

void MenuTabsAimbot(struct AimConfig* aim, struct CheatConfig* cfg)
{
    if (!aim || !cfg)
        return;
    cfg->aim = *aim;
    ApplyLabels(g_aimLabels, (int)(sizeof(g_aimLabels) / sizeof(g_aimLabels[0])), cfg);
    *aim = cfg->aim;
}

void MenuTabsSkillCheck(struct SkillCheckConfig* sc)
{
    struct CheatConfig tmp;
    if (!sc)
        return;
    memset(&tmp, 0, sizeof(tmp));
    tmp.skillCheck = *sc;
    ApplyLabels(g_skillLabels, (int)(sizeof(g_skillLabels) / sizeof(g_skillLabels[0])), &tmp);
    *sc = tmp.skillCheck;
}

void MenuTabsCrosshair(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    ApplyLabels(g_crosshairLabels, (int)(sizeof(g_crosshairLabels) / sizeof(g_crosshairLabels[0])), cfg);
}

void MenuTabsHitsound(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    ApplyLabels(g_hitsoundLabels, (int)(sizeof(g_hitsoundLabels) / sizeof(g_hitsoundLabels[0])), cfg);
}

void MenuTabsFovSpeed(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    ApplyLabels(g_speedLabels, (int)(sizeof(g_speedLabels) / sizeof(g_speedLabels[0])), cfg);
}

void MenuTabsMoonwalk(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    ApplyLabels(g_moonwalkLabels, (int)(sizeof(g_moonwalkLabels) / sizeof(g_moonwalkLabels[0])), cfg);
}

void MenuTabsConfigs(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    MenuStyleClampTheme(&cfg->theme);
}

void MenuTabsWatermarks(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    ApplyLabels(g_watermarkLabels, (int)(sizeof(g_watermarkLabels) / sizeof(g_watermarkLabels[0])), cfg);
}

void MenuTabsStreamproof(struct CheatConfig* cfg)
{
    if (!cfg)
        return;
    ApplyLabels(g_moonwalkLabels, (int)(sizeof(g_moonwalkLabels) / sizeof(g_moonwalkLabels[0])), cfg);
}

void MenuTabsSidebar(struct CheatConfig* cfg, int activeTab)
{
    if (!cfg)
        return;
    if (activeTab < 0 || activeTab >= MenuTabCount)
        return;
    MenuStyleClampTheme(&cfg->theme);
}

void MenuTabsDispatch(struct CheatConfig* cfg, int activeTab)
{
    if (!cfg)
        return;
    switch (activeTab) {
    case MenuTabEsp:
        MenuTabsEsp(&cfg->esp);
        break;
    case MenuTabAuras:
        MenuTabsAuras(&cfg->aura);
        break;
    case MenuTabAimbot:
        MenuTabsAimbot(&cfg->aim, cfg);
        break;
    case MenuTabSkillCheck:
        MenuTabsSkillCheck(&cfg->skillCheck);
        break;
    case MenuTabCrosshair:
        MenuTabsCrosshair(cfg);
        break;
    case MenuTabHitsound:
        MenuTabsHitsound(cfg);
        break;
    case MenuTabBlindBar:
        ApplyLabels(g_miscLabels, (int)(sizeof(g_miscLabels) / sizeof(g_miscLabels[0])), cfg);
        break;
    case MenuTabFovSpeed:
        MenuTabsFovSpeed(cfg);
        break;
    case MenuTabMoonwalk:
        MenuTabsMoonwalk(cfg);
        break;
    case MenuTabConfigs:
        MenuTabsConfigs(cfg);
        break;
    case MenuTabWatermarks:
        MenuTabsWatermarks(cfg);
        break;
    case MenuTabStreamproof:
        MenuTabsStreamproof(cfg);
        break;
    case MenuTabSidebar:
    default:
        MenuTabsSidebar(cfg, activeTab);
        ApplyLabels(g_miscLabels, (int)(sizeof(g_miscLabels) / sizeof(g_miscLabels[0])), cfg);
        break;
    }
}
