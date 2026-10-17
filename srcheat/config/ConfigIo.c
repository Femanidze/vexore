#include "ConfigIo.h"
#include "../core/Globals.h"
#include "ConfigLoad.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <shlobj.h>

const char* ConfigIoDefaultName(void)
{
    return "default" VEX_CONFIG_EXT;
}

int ConfigIoPickDirectory(char* outDir, int cap)
{
    wchar_t wdocs[MAX_PATH];
    char docs[MAX_PATH];
    char narrow[MAX_PATH];
    DWORD n;

    if (GetFileAttributesA(VEX_CONFIG_DIR_FALLBACK) != 0xFFFFFFFF &&
        (GetFileAttributesA(VEX_CONFIG_DIR_FALLBACK) & 0x10)) {
        snprintf(outDir, cap, "%s", VEX_CONFIG_DIR_FALLBACK);
        return 1;
    }
    if (!SHGetFolderPathW(NULL, 0x0005, NULL, 0, wdocs)) {
        n = (DWORD)WideCharToMultiByte(CP_ACP, 0, wdocs, -1, narrow, MAX_PATH, NULL, NULL);
        if (n > 1) {
            char appData[MAX_PATH];
            snprintf(appData, sizeof(appData), "%s\\%s", narrow, VEX_CONFIG_DIR_DOCUMENTS);
            if (ConfigEnsureDirectory(appData)) {
                snprintf(outDir, cap, "%s", appData);
                return 1;
            }
            snprintf(outDir, cap, "%s", narrow);
            return 1;
        }
    }
    n = GetEnvironmentVariableA("USERPROFILE", docs, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
        char appData[MAX_PATH];
        snprintf(appData, sizeof(appData), "%s\\Documents", docs);
        if (GetFileAttributesA(appData) != 0xFFFFFFFF) {
            snprintf(outDir, cap, "%s", appData);
            return 1;
        }
    }
    MessageBoxA(NULL, VEX_MSG_ACCESS_DENIED, VEX_CONFIG_TITLE_ACCESS, 0x124);
    if (ConfigEnsureDirectory(VEX_CONFIG_DIR_FALLBACK)) {
        snprintf(outDir, cap, "%s", VEX_CONFIG_DIR_FALLBACK);
        return 1;
    }
    snprintf(outDir, cap, "%s", ".\\");
    return 0;
}

int ConfigIoLoad(const char* path, struct CheatConfig* cfg)
{
    int st;

    if (!path || !cfg)
        return ConfigMissing;
    st = ConfigLoadFromFile(path, cfg);
    if (st == ConfigCorrupted)
        MessageBoxA(NULL, "Config file is corrupted and cannot be parsed.", VEX_CONFIG_TITLE_CORRUPT, 0x10);
    else if (st == ConfigMissing)
        MessageBoxA(NULL, "Config file was not found.", VEX_CONFIG_TITLE_LOAD, 0x30);
    return st;
}

int ConfigIoSave(const char* path, const struct CheatConfig* cfg)
{
    int st;
    DWORD attr;

    if (!path || !cfg)
        return ConfigWriteFailed;
    attr = GetFileAttributesA(path);
    if (attr != 0xFFFFFFFF && (attr & 0x1)) {
        MessageBoxA(NULL, "Cannot save config: File has 'Read-Only' attribute. Remove it in file properties.", VEX_CONFIG_TITLE_SAVE, 0x10);
        return ConfigAccessDenied;
    }
    st = ConfigSaveToFile(path, cfg);
    if (st == ConfigAccessDenied) {
        if (MessageBoxA(NULL, VEX_MSG_WRITE_DENIED, VEX_CONFIG_TITLE_WRITE_ACCESS, 0x24) == 6) {
            char alt[MAX_PATH];
            snprintf(alt, sizeof(alt), "%s\\%s", VEX_CONFIG_DIR_FALLBACK, ConfigIoDefaultName());
            if (ConfigEnsureDirectory(VEX_CONFIG_DIR_FALLBACK))
                st = ConfigSaveToFile(alt, cfg);
        }
    }
    if (st != ConfigOk)
        MessageBoxA(NULL, "Failed to write config file to disk.", VEX_CONFIG_TITLE_WRITE_FAILED, 0x10);
    return st;
}
