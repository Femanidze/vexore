#include "ConfigLoad.h"
#include "../core/Globals.h"
#include "ConfigKeys.h"
#include "JsonParser.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int IsSep(wchar_t ch)
{
    return ch == L'\\' || ch == L'/';
}

static void AppendSepIfNeeded(wchar_t* dst, int* len, int cap)
{
    if (*len <= 0 || *len >= cap - 1)
        return;
    if (!IsSep(dst[*len - 1])) {
        dst[*len] = L'\\';
        (*len)++;
        dst[*len] = 0;
    }
}

int ConfigBuildDocumentPath(const char* fileName, struct ConfigPathResult* out)
{
    wchar_t docs[MAX_PATH];
    wchar_t full[MAX_PATH];
    int len;
    int flen;

    memset(out, 0, sizeof(*out));
    if (!SHGetFolderPathW(NULL, 0x0005, NULL, 0, docs))
        return 0;
    if (!docs[0])
        return 0;
    len = (int)wcslen(docs);
    AppendSepIfNeeded(docs, &len, MAX_PATH);
    flen = (int)MultiByteToWideChar(CP_ACP, 0, fileName, -1, NULL, 0);
    if (flen <= 1 || len + flen >= MAX_PATH)
        return 0;
    wcsncpy(full, docs, MAX_PATH);
    MultiByteToWideChar(CP_ACP, 0, fileName, -1, full + len, MAX_PATH - len);
    WideCharToMultiByte(CP_ACP, 0, full, -1, out->path, sizeof(out->path), NULL, NULL);
    out->status = ConfigOk;
    out->fallbackUsed = 0;
    return 1;
}

int ConfigBuildFallbackPath(const char* fileName, struct ConfigPathResult* out)
{
    memset(out, 0, sizeof(*out));
    snprintf(out->path, sizeof(out->path), "%s\\%s", VEX_CONFIG_DIR_FALLBACK, fileName);
    out->status = ConfigOk;
    out->fallbackUsed = 1;
    return 1;
}

int ConfigEnsureDirectory(const char* dirPath)
{
    DWORD attr = GetFileAttributesA(dirPath);
    if (attr != 0xFFFFFFFF)
        return (attr & 0x10) ? 1 : 0;
    if (CreateDirectoryA(dirPath, NULL))
        return 1;
    return GetLastError() == 183 ? 1 : 0;
}

int ConfigSelectExisting(const char* preferredDir, const char* fallbackDir, char* outPath, int outCap)
{
    if (preferredDir && preferredDir[0]) {
        if (GetFileAttributesA(preferredDir) != 0xFFFFFFFF) {
            snprintf(outPath, outCap, "%s", preferredDir);
            return 1;
        }
    }
    if (fallbackDir && fallbackDir[0]) {
        if (GetFileAttributesA(fallbackDir) != 0xFFFFFFFF) {
            snprintf(outPath, outCap, "%s", fallbackDir);
            return 1;
        }
    }
    if (fallbackDir && ConfigEnsureDirectory(fallbackDir)) {
        snprintf(outPath, outCap, "%s", fallbackDir);
        return 1;
    }
    return 0;
}

static void ApplyJsonObject(const struct JsonObject* j, struct CheatConfig* cfg)
{
    struct ColorRgba col;

    cfg->esp.enabled = JsonGetBool(j, KEY_ESP_ENABLED, cfg->esp.enabled);
    cfg->esp.lerpSmooth = JsonGetFloat(j, KEY_ESP_LERP, cfg->esp.lerpSmooth);
    cfg->esp.maxDistance = JsonGetFloat(j, KEY_ESP_MAX_DIST, cfg->esp.maxDistance);
    cfg->esp.boxType = JsonGetInt(j, KEY_ESP_BOX_TYPE, cfg->esp.boxType);
    cfg->esp.boxOutline = JsonGetBool(j, KEY_ESP_BOX_OUTLINE, cfg->esp.boxOutline);
    cfg->esp.boxFill = JsonGetBool(j, KEY_ESP_BOX_FILL, cfg->esp.boxFill);
    cfg->esp.boxFillAlpha = JsonGetFloat(j, KEY_ESP_BOX_FILL_ALPHA, cfg->esp.boxFillAlpha);
    cfg->esp.boxGradient = JsonGetBool(j, KEY_ESP_BOX_GRADIENT, cfg->esp.boxGradient);
    cfg->esp.boxThickness = JsonGetFloat(j, KEY_ESP_BOX_THICKNESS, cfg->esp.boxThickness);
    cfg->esp.boxDistanceFade = JsonGetFloat(j, KEY_ESP_BOX_DIST_FADE, cfg->esp.boxDistanceFade);
    cfg->esp.showHealthBar = JsonGetBool(j, KEY_ESP_SHOW_HEALTH, cfg->esp.showHealthBar);
    cfg->esp.showRecoverBar = JsonGetBool(j, KEY_ESP_SHOW_RECOVER, cfg->esp.showRecoverBar);
    cfg->esp.nameUppercase = JsonGetBool(j, KEY_ESP_NAME_UPPER, cfg->esp.nameUppercase);
    cfg->esp.nameShadow = JsonGetBool(j, KEY_ESP_NAME_SHADOW, cfg->esp.nameShadow);
    cfg->esp.nameScale = JsonGetFloat(j, KEY_ESP_NAME_SCALE, cfg->esp.nameScale);
    cfg->esp.nicknameOffX = JsonGetFloat(j, KEY_ESP_NICK_OFF_X, cfg->esp.nicknameOffX);
    cfg->esp.nicknameOffY = JsonGetFloat(j, KEY_ESP_NICK_OFF_Y, cfg->esp.nicknameOffY);
    cfg->esp.nameAngle = JsonGetFloat(j, KEY_ESP_NAME_ANGLE, cfg->esp.nameAngle);
    cfg->esp.skeletonType = JsonGetInt(j, KEY_ESP_SKELETON_TYPE, cfg->esp.skeletonType);
    cfg->esp.skeletonDots = JsonGetBool(j, KEY_ESP_SKELETON_DOTS, cfg->esp.skeletonDots);
    if (JsonGetColor(j, KEY_COL_BOX_SURV, &col))
        cfg->esp.colBoxSurv = col;
    if (JsonGetColor(j, KEY_COL_BOX_KILLER, &col))
        cfg->esp.colBoxKiller = col;
    if (JsonGetColor(j, KEY_COL_BOX_HEALTHY, &col))
        cfg->esp.colBoxHealthy = col;
    if (JsonGetColor(j, KEY_COL_BOX_INJURED, &col))
        cfg->esp.colBoxInjured = col;
    if (JsonGetColor(j, KEY_COL_BOX_DOWNED, &col))
        cfg->esp.colBoxDowned = col;
    if (JsonGetColor(j, KEY_COL_ESP_NAME, &col))
        cfg->esp.colName = col;
    if (JsonGetColor(j, KEY_ESP_BOX_HEALTH_COL, &col))
        cfg->esp.colHealthBar = col;

    cfg->aim.enabled = JsonGetBool(j, KEY_AIM_ENABLED, cfg->aim.enabled);
    cfg->aim.key = JsonGetInt(j, KEY_AIM_KEY, cfg->aim.key);
    cfg->aim.keyMode = JsonGetInt(j, KEY_AIM_KEY_MODE, cfg->aim.keyMode);
    cfg->aim.fovRadius = JsonGetFloat(j, KEY_AIM_FOV_RADIUS, cfg->aim.fovRadius);
    cfg->aim.smoothing = JsonGetFloat(j, KEY_AIM_SMOOTHING, cfg->aim.smoothing);
    cfg->aim.bonePriority[0] = JsonGetInt(j, KEY_AIM_BONE_PRIO0, cfg->aim.bonePriority[0]);
    cfg->aim.bonePriority[1] = JsonGetInt(j, KEY_AIM_BONE_PRIO1, cfg->aim.bonePriority[1]);
    cfg->aim.bonePriority[2] = JsonGetInt(j, KEY_AIM_BONE_PRIO2, cfg->aim.bonePriority[2]);
    cfg->aim.bonePriority[3] = JsonGetInt(j, KEY_AIM_BONE_PRIO3, cfg->aim.bonePriority[3]);

    cfg->skillCheck.enabled = JsonGetBool(j, KEY_SKILLCHECK_ENABLED, cfg->skillCheck.enabled);
    cfg->skillCheck.zoneMode = JsonGetInt(j, KEY_SKILLCHECK_ZONE, cfg->skillCheck.zoneMode);
    cfg->skillCheck.greatChance = JsonGetFloat(j, KEY_SKILLCHECK_GREAT, cfg->skillCheck.greatChance);

    cfg->speedHack.enabled = JsonGetBool(j, KEY_SPEEDHACK_ENABLED, cfg->speedHack.enabled);
    cfg->speedHack.value = JsonGetFloat(j, KEY_SPEEDHACK_VALUE, cfg->speedHack.value);

    cfg->aura.masterSwitch = JsonGetBool(j, KEY_AURA_MASTER, cfg->aura.masterSwitch);
    cfg->aura.hatch.enabled = JsonGetBool(j, KEY_AURA_HATCH_EN, cfg->aura.hatch.enabled);
    cfg->aura.hatch.distance = JsonGetFloat(j, KEY_AURA_HATCH_DIST, cfg->aura.hatch.distance);
    if (JsonGetColor(j, KEY_AURA_COLOR_HATCH, &col))
        cfg->aura.hatch.color = col;
    cfg->aura.hooks.enabled = JsonGetBool(j, KEY_AURA_HOOKS_EN, cfg->aura.hooks.enabled);
    cfg->aura.hooks.distance = JsonGetFloat(j, KEY_AURA_HOOKS_DIST, cfg->aura.hooks.distance);
    if (JsonGetColor(j, KEY_AURA_COLOR_HOOKS, &col))
        cfg->aura.hooks.color = col;
    cfg->aura.chests.enabled = JsonGetBool(j, KEY_AURA_CHESTS_EN, cfg->aura.chests.enabled);
    cfg->aura.chests.distance = JsonGetFloat(j, KEY_AURA_CHESTS_DIST, cfg->aura.chests.distance);
    if (JsonGetColor(j, KEY_AURA_COLOR_CHESTS, &col))
        cfg->aura.chests.color = col;
    cfg->aura.totems.enabled = JsonGetBool(j, KEY_AURA_TOTEMS_EN, cfg->aura.totems.enabled);
    cfg->aura.totems.distance = JsonGetFloat(j, KEY_AURA_TOTEMS_DIST, cfg->aura.totems.distance);
    if (JsonGetColor(j, KEY_AURA_COLOR_TOTEMS, &col))
        cfg->aura.totems.color = col;
    cfg->aura.pallets.enabled = JsonGetBool(j, KEY_AURA_PALLET_EN, cfg->aura.pallets.enabled);
    cfg->aura.pallets.distance = JsonGetFloat(j, KEY_AURA_PALLET_DIST, cfg->aura.pallets.distance);
    if (JsonGetColor(j, KEY_COL_PALLET_DEFAULT, &col))
        cfg->aura.pallets.color = col;
    cfg->aura.trapsMaster = JsonGetBool(j, KEY_AURA_TRAPS_MASTER, cfg->aura.trapsMaster);
    cfg->aura.trapsDistance = JsonGetFloat(j, KEY_AURA_TRAPS_DIST, cfg->aura.trapsDistance);
    cfg->aura.trapBear = JsonGetBool(j, KEY_AURA_TRAP_BEAR, cfg->aura.trapBear);
    cfg->aura.trapZombie = JsonGetBool(j, KEY_AURA_TRAP_ZOMBIE, cfg->aura.trapZombie);
    if (JsonGetColor(j, KEY_AURA_COLOR_TRAP_BEAR, &col))
        cfg->aura.colorTrapBear = col;
    if (JsonGetColor(j, KEY_AURA_COLOR_TRAP_ZOMBIE, &col))
        cfg->aura.colorTrapZombie = col;
    cfg->aura.windowAuraMaxDist = JsonGetFloat(j, KEY_WINDOW_AURA_MAX, cfg->aura.windowAuraMaxDist);

    cfg->theme.r = JsonGetFloat(j, KEY_MENU_THEME_R, cfg->theme.r);
    cfg->theme.g = JsonGetFloat(j, KEY_MENU_THEME_G, cfg->theme.g);
    cfg->theme.b = JsonGetFloat(j, KEY_MENU_THEME_B, cfg->theme.b);
    cfg->theme.alpha = JsonGetFloat(j, KEY_MENU_THEME_ALPHA, cfg->theme.alpha);
}

int ConfigLoadFromFile(const char* path, struct CheatConfig* cfg)
{
    FILE* f;
    long size;
    char* buf;
    size_t rd;
    struct JsonObject obj;

    if (!path || !cfg)
        return ConfigMissing;
    f = fopen(path, "rb");
    if (!f)
        return ConfigMissing;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0 || size > 1 << 20) {
        fclose(f);
        return ConfigCorrupted;
    }
    buf = (char*)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return ConfigWriteFailed;
    }
    rd = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (rd != (size_t)size) {
        free(buf);
        return ConfigCorrupted;
    }
    buf[rd] = 0;
    memset(&obj, 0, sizeof(obj));
    if (!JsonParseObject(buf, (long long)rd, &obj)) {
        free(buf);
        return ConfigCorrupted;
    }
    ApplyJsonObject(&obj, cfg);
    JsonFreeObject(&obj);
    free(buf);
    return ConfigOk;
}

static int WriteColorPair(struct JsonWriter* w, const char* key, const struct ColorRgba* c)
{
    if (!JsonWriterKey(w, key))
        return 0;
    return JsonWriterColor(w, c);
}

int ConfigSaveToFile(const char* path, const struct CheatConfig* cfg)
{
    FILE* f;
    struct JsonWriter w;
    size_t wr;

    if (!path || !cfg)
        return ConfigWriteFailed;
    if (!JsonWriterInit(&w, 8192))
        return ConfigWriteFailed;
    if (!JsonWriterBeginObject(&w))
        goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_ENABLED) || !JsonWriterBool(&w, cfg->esp.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_LERP) || !JsonWriterFloat(&w, cfg->esp.lerpSmooth)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_MAX_DIST) || !JsonWriterFloat(&w, cfg->esp.maxDistance)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_BOX_TYPE) || !JsonWriterInt(&w, cfg->esp.boxType)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_BOX_OUTLINE) || !JsonWriterBool(&w, cfg->esp.boxOutline)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_BOX_FILL) || !JsonWriterBool(&w, cfg->esp.boxFill)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_BOX_FILL_ALPHA) || !JsonWriterFloat(&w, cfg->esp.boxFillAlpha)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_BOX_GRADIENT) || !JsonWriterBool(&w, cfg->esp.boxGradient)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_BOX_THICKNESS) || !JsonWriterFloat(&w, cfg->esp.boxThickness)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_BOX_DIST_FADE) || !JsonWriterFloat(&w, cfg->esp.boxDistanceFade)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_SHOW_HEALTH) || !JsonWriterBool(&w, cfg->esp.showHealthBar)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_SHOW_RECOVER) || !JsonWriterBool(&w, cfg->esp.showRecoverBar)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_NAME_UPPER) || !JsonWriterBool(&w, cfg->esp.nameUppercase)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_NAME_SHADOW) || !JsonWriterBool(&w, cfg->esp.nameShadow)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_NAME_SCALE) || !JsonWriterFloat(&w, cfg->esp.nameScale)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_NICK_OFF_X) || !JsonWriterFloat(&w, cfg->esp.nicknameOffX)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_NICK_OFF_Y) || !JsonWriterFloat(&w, cfg->esp.nicknameOffY)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_NAME_ANGLE) || !JsonWriterFloat(&w, cfg->esp.nameAngle)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_SKELETON_TYPE) || !JsonWriterInt(&w, cfg->esp.skeletonType)) goto fail;
    if (!JsonWriterKey(&w, KEY_ESP_SKELETON_DOTS) || !JsonWriterBool(&w, cfg->esp.skeletonDots)) goto fail;
    if (!WriteColorPair(&w, KEY_COL_BOX_SURV, &cfg->esp.colBoxSurv)) goto fail;
    if (!WriteColorPair(&w, KEY_COL_BOX_KILLER, &cfg->esp.colBoxKiller)) goto fail;
    if (!WriteColorPair(&w, KEY_COL_BOX_HEALTHY, &cfg->esp.colBoxHealthy)) goto fail;
    if (!WriteColorPair(&w, KEY_COL_BOX_INJURED, &cfg->esp.colBoxInjured)) goto fail;
    if (!WriteColorPair(&w, KEY_COL_BOX_DOWNED, &cfg->esp.colBoxDowned)) goto fail;
    if (!WriteColorPair(&w, KEY_COL_ESP_NAME, &cfg->esp.colName)) goto fail;
    if (!WriteColorPair(&w, KEY_ESP_BOX_HEALTH_COL, &cfg->esp.colHealthBar)) goto fail;

    if (!JsonWriterKey(&w, KEY_AIM_ENABLED) || !JsonWriterBool(&w, cfg->aim.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_AIM_KEY) || !JsonWriterInt(&w, cfg->aim.key)) goto fail;
    if (!JsonWriterKey(&w, KEY_AIM_KEY_MODE) || !JsonWriterInt(&w, cfg->aim.keyMode)) goto fail;
    if (!JsonWriterKey(&w, KEY_AIM_FOV_RADIUS) || !JsonWriterFloat(&w, cfg->aim.fovRadius)) goto fail;
    if (!JsonWriterKey(&w, KEY_AIM_SMOOTHING) || !JsonWriterFloat(&w, cfg->aim.smoothing)) goto fail;
    if (!JsonWriterKey(&w, KEY_AIM_BONE_PRIO0) || !JsonWriterInt(&w, cfg->aim.bonePriority[0])) goto fail;
    if (!JsonWriterKey(&w, KEY_AIM_BONE_PRIO1) || !JsonWriterInt(&w, cfg->aim.bonePriority[1])) goto fail;
    if (!JsonWriterKey(&w, KEY_AIM_BONE_PRIO2) || !JsonWriterInt(&w, cfg->aim.bonePriority[2])) goto fail;
    if (!JsonWriterKey(&w, KEY_AIM_BONE_PRIO3) || !JsonWriterInt(&w, cfg->aim.bonePriority[3])) goto fail;

    if (!JsonWriterKey(&w, KEY_SKILLCHECK_ENABLED) || !JsonWriterBool(&w, cfg->skillCheck.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_SKILLCHECK_ZONE) || !JsonWriterInt(&w, cfg->skillCheck.zoneMode)) goto fail;
    if (!JsonWriterKey(&w, KEY_SKILLCHECK_GREAT) || !JsonWriterFloat(&w, cfg->skillCheck.greatChance)) goto fail;

    if (!JsonWriterKey(&w, KEY_SPEEDHACK_ENABLED) || !JsonWriterBool(&w, cfg->speedHack.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_SPEEDHACK_VALUE) || !JsonWriterFloat(&w, cfg->speedHack.value)) goto fail;

    if (!JsonWriterKey(&w, KEY_AURA_MASTER) || !JsonWriterBool(&w, cfg->aura.masterSwitch)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_HATCH_EN) || !JsonWriterBool(&w, cfg->aura.hatch.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_HATCH_DIST) || !JsonWriterFloat(&w, cfg->aura.hatch.distance)) goto fail;
    if (!WriteColorPair(&w, KEY_AURA_COLOR_HATCH, &cfg->aura.hatch.color)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_HOOKS_EN) || !JsonWriterBool(&w, cfg->aura.hooks.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_HOOKS_DIST) || !JsonWriterFloat(&w, cfg->aura.hooks.distance)) goto fail;
    if (!WriteColorPair(&w, KEY_AURA_COLOR_HOOKS, &cfg->aura.hooks.color)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_CHESTS_EN) || !JsonWriterBool(&w, cfg->aura.chests.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_CHESTS_DIST) || !JsonWriterFloat(&w, cfg->aura.chests.distance)) goto fail;
    if (!WriteColorPair(&w, KEY_AURA_COLOR_CHESTS, &cfg->aura.chests.color)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_TOTEMS_EN) || !JsonWriterBool(&w, cfg->aura.totems.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_TOTEMS_DIST) || !JsonWriterFloat(&w, cfg->aura.totems.distance)) goto fail;
    if (!WriteColorPair(&w, KEY_AURA_COLOR_TOTEMS, &cfg->aura.totems.color)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_PALLET_EN) || !JsonWriterBool(&w, cfg->aura.pallets.enabled)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_PALLET_DIST) || !JsonWriterFloat(&w, cfg->aura.pallets.distance)) goto fail;
    if (!WriteColorPair(&w, KEY_COL_PALLET_DEFAULT, &cfg->aura.pallets.color)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_TRAPS_MASTER) || !JsonWriterBool(&w, cfg->aura.trapsMaster)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_TRAPS_DIST) || !JsonWriterFloat(&w, cfg->aura.trapsDistance)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_TRAP_BEAR) || !JsonWriterBool(&w, cfg->aura.trapBear)) goto fail;
    if (!JsonWriterKey(&w, KEY_AURA_TRAP_ZOMBIE) || !JsonWriterBool(&w, cfg->aura.trapZombie)) goto fail;
    if (!WriteColorPair(&w, KEY_AURA_COLOR_TRAP_BEAR, &cfg->aura.colorTrapBear)) goto fail;
    if (!WriteColorPair(&w, KEY_AURA_COLOR_TRAP_ZOMBIE, &cfg->aura.colorTrapZombie)) goto fail;
    if (!JsonWriterKey(&w, KEY_WINDOW_AURA_MAX) || !JsonWriterFloat(&w, cfg->aura.windowAuraMaxDist)) goto fail;

    if (!JsonWriterKey(&w, KEY_MENU_THEME_R) || !JsonWriterFloat(&w, cfg->theme.r)) goto fail;
    if (!JsonWriterKey(&w, KEY_MENU_THEME_G) || !JsonWriterFloat(&w, cfg->theme.g)) goto fail;
    if (!JsonWriterKey(&w, KEY_MENU_THEME_B) || !JsonWriterFloat(&w, cfg->theme.b)) goto fail;
    if (!JsonWriterKey(&w, KEY_MENU_THEME_ALPHA) || !JsonWriterFloat(&w, cfg->theme.alpha)) goto fail;
    if (!JsonWriterEndObject(&w))
        goto fail;

    f = fopen(path, "wb");
    if (!f) {
        JsonWriterFree(&w);
        return ConfigAccessDenied;
    }
    wr = fwrite(w.buf, 1, (size_t)w.len, f);
    fclose(f);
    {
        long long savedLen = w.len;
        JsonWriterFree(&w);
        if (wr != (size_t)savedLen)
            return ConfigWriteFailed;
    }
    return ConfigOk;
fail:
    JsonWriterFree(&w);
    return ConfigCorrupted;
}

int ConfigDeleteFile(const char* path)
{
    if (!path)
        return ConfigAccessDenied;
    if (DeleteFileA(path))
        return ConfigOk;
    if (GetLastError() == 2)
        return ConfigOk;
    return ConfigAccessDenied;
}

int ConfigRenameFile(const char* oldPath, const char* newPath)
{
    if (!oldPath || !newPath)
        return ConfigAccessDenied;
    if (MoveFileA(oldPath, newPath))
        return ConfigOk;
    return ConfigAccessDenied;
}
