#include "Threads.h"
#include "Globals.h"
#include "Timing.h"
#include "../game/LobbyWatcher.h"
#include "../features/GameRuntime.h"
#include "../features/Features.h"

#include <windows.h>

static HANDLE g_hFeature;
static HANDLE g_hSignature;
static HANDLE g_hLobby;

unsigned long __stdcall FeatureLoopThread(void* arg)
{
    long long now;
    long long target;

    (void)arg;
    while (g_runningFlag) {
        if (g_gameBase)
            GameFeatureTick();
        now = TimingNowNs();
        if (now > 0x7FFFFFFFE2329AFE0LL)
            target = 0x7FFFFFFFFFFFFFFFLL;
        else
            target = now + 500000000LL;
        TimingSleepUntilNs(target);
    }
    return 0;
}

unsigned long __stdcall SignatureLoopThread(void* arg)
{
    long long now;
    long long target;

    (void)arg;
    while (g_runningFlag) {
        if (g_gameBase)
            SignatureScanTick((unsigned int)g_moduleHandle, g_espWindowWidth);
        now = TimingNowNs();
        if (now > 0x7FFFFFFFFFB3B4BEL)
            target = 0x7FFFFFFFFFFFFFFFLL;
        else
            target = now + 5000000LL;
        TimingSleepUntilNs(target);
    }
    return 0;
}

unsigned long __stdcall LobbyThread(void* arg)
{
    (void)arg;
    LobbyWatcherLoop();
    return 0;
}

void ThreadStartAll(void)
{
    g_runningFlag = 1;
    g_hFeature = CreateThread(NULL, 0, FeatureLoopThread, NULL, 0, NULL);
    g_hSignature = CreateThread(NULL, 0, SignatureLoopThread, NULL, 0, NULL);
    g_hLobby = CreateThread(NULL, 0, LobbyThread, NULL, 0, NULL);
}

void ThreadStopAll(void)
{
    g_runningFlag = 0;
    if (g_hFeature) {
        WaitForSingleObject(g_hFeature, 3000);
        CloseHandle(g_hFeature);
        g_hFeature = NULL;
    }
    if (g_hSignature) {
        WaitForSingleObject(g_hSignature, 3000);
        CloseHandle(g_hSignature);
        g_hSignature = NULL;
    }
    if (g_hLobby) {
        WaitForSingleObject(g_hLobby, 3000);
        CloseHandle(g_hLobby);
        g_hLobby = NULL;
    }
}
