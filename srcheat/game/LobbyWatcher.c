#include "LobbyWatcher.h"
#include "../core/Globals.h"
#include "../core/Timing.h"

#include <windows.h>
#include <string.h>

static HWND FindGameWindow(void)
{
    HWND hw = NULL;
    for (;;) {
        hw = FindWindowExA(NULL, hw, VEX_WIN_CLASS_GAME, VEX_WIN_TITLE_GAME);
        if (!hw)
            break;
        return hw;
    }
    return NULL;
}

static int ReadPtr(qword addr, qword* out)
{
    if (addr < 0x10000)
        return 0;
    if (addr - 0x10000 > 0x7FFFFEFFFFFFULL)
        return 0;
    if (!g_moduleHandle)
        return 0;
    if (!IsBadReadPtr((const void*)addr, sizeof(qword)))
        *out = *(const qword*)addr;
    else
        return 0;
    return 1;
}

int LobbyWatcherFindGameWindow(void)
{
    if (!FindGameWindow())
        return 0;
    return 1;
}

int LobbyWatcherResolveBase(void)
{
    HMODULE mod;
    qword base;
    SIZE_T modSize = 0;

    if (!g_moduleHandle)
        return 0;
    mod = (HMODULE)(uintptr_t)g_moduleHandle;
    base = (qword)(uintptr_t)mod;
    if (!base)
        return 0;
    if ((base + modSize - 0x1000000) > 0x7FFFFEFFFFFFULL)
        return 0;
    g_gameBase = base;
    return 1;
}

int LobbyWatcherValidateChain(void)
{
    qword cur;
    qword next;

    cur = g_gameBase;
    if (!cur)
        return 0;
    if (!ReadPtr(cur + 592, &next) || next < 0x10000)
        return 0;
    if (next - 0x10000 > 0x7FFFFEFFFFFFULL)
        return 0;
    cur = next;
    if (!ReadPtr(cur + 88, &next) || next < 0x10000)
        return 0;
    if (next - 0x1000000 > 0x7FFFFEFFFFFFULL)
        return 0;
    cur = next;
    if (!ReadPtr(cur + 80, &next) || next < 0x10000)
        return 0;
    if (next - 0x1000000 > 0x7FFFFEFFFFFFULL)
        return 0;
    cur = next;
    if (!ReadPtr(cur + 944, &next) || next < 0x10000)
        return 0;
    g_connected = 1;
    g_baseCemented = 1;
    return 1;
}

static void SleepNsCap(long long raw, long long addNs, long long limit)
{
    long long target;
    if (raw > limit)
        target = 0x7FFFFFFFFFFFFFFFLL;
    else
        target = raw + addNs;
    TimingSleepUntilNs(target);
}

void LobbyWatcherLoop(void)
{
    while (g_runningFlag) {
        if (g_baseCemented) {
            if (LobbyWatcherValidateChain())
                TimingSleepDeltaNs(1000000000LL);
            else
                TimingSleepDeltaNs(500000000LL);
            continue;
        }
        if (!LobbyWatcherFindGameWindow()) {
            TimingSleepDeltaNs(5000000000LL);
            continue;
        }
        if (g_moduleHandle && !g_gameBase) {
            if (LobbyWatcherResolveBase() && LobbyWatcherValidateChain()) {
                g_connected = 1;
            } else {
                g_moduleHandle = 0;
                g_gameBase = 0;
                g_signatureOffA = 0;
                g_signatureOffB = 0;
            }
        } else if (g_gameBase && !g_baseCemented) {
            if (LobbyWatcherValidateChain())
                g_baseCemented = 1;
        }
        TimingSleepDeltaNs(500000000LL);
    }
}
