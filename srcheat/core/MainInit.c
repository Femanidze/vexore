#include "MainInit.h"
#include "Globals.h"
#include "../config/ConfigDefaults.h"
#include "../config/ConfigIo.h"
#include "../config/ConfigLoad.h"
#include "Threads.h"
#include "../features/Features.h"
#include "../game/LobbyWatcher.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>

void MainInitConfig(void)
{
    char dir[260];
    char path[300];

    ConfigInit(&g_config);
    memset(path, 0, sizeof(path));
    if (ConfigIoPickDirectory(dir, (int)sizeof(dir))) {
        snprintf(path, sizeof(path), "%s\\%s%s", dir, "default", VEX_CONFIG_EXT);
        if (GetFileAttributesA(path) != 0xFFFFFFFF)
            ConfigIoLoad(path, &g_config);
    }
}

void MainInitThreads(void)
{
    ThreadStartAll();
}

int MainInitAll(void)
{
    g_runningFlag = 1;
    g_connected = 0;
    g_baseCemented = 0;
    g_gameBase = 0;
    g_signatureOffA = 0;
    g_signatureOffB = 0;
    MainInitConfig();
    FeatureApplyAll(&g_config);
    MainInitThreads();
    return 1;
}

void MainShutdown(void)
{
    char dir[260];
    char path[300];

    FeatureRestoreAll();
    ThreadStopAll();
    if (ConfigIoPickDirectory(dir, (int)sizeof(dir))) {
        snprintf(path, sizeof(path), "%s\\%s%s", dir, "default", VEX_CONFIG_EXT);
        ConfigIoSave(path, &g_config);
    }
    g_runningFlag = 0;
    g_connected = 0;
    g_baseCemented = 0;
    g_gameBase = 0;
}
