#include "Timing.h"

#include <windows.h>

static long long g_freq = 0;
static long long g_baseCount = 0;

static long long QueryRaw(void)
{
    LARGE_INTEGER li;
    if (!g_freq) {
        QueryPerformanceFrequency(&li);
        g_freq = li.QuadPart;
        QueryPerformanceCounter(&li);
        g_baseCount = li.QuadPart;
        return 0;
    }
    QueryPerformanceCounter(&li);
    if (g_freq == 10000000)
        return 100LL * (li.QuadPart - g_baseCount);
    if (g_freq == 24000000) {
        long long d = li.QuadPart - g_baseCount;
        return 41LL * d + (2LL * d) / 3;
    }
    {
        long long d = li.QuadPart - g_baseCount;
        return 1000000000LL * (d / g_freq) + 1000000000LL * (d % g_freq) / g_freq;
    }
}

long long TimingNowNs(void)
{
    return QueryRaw();
}

void TimingSleepUntilNs(long long targetNs)
{
    long long now = QueryRaw();
    long long delta;
    DWORD ms;

    if (targetNs == now || targetNs < now)
        return;
    delta = targetNs - now;
    if (delta >= 0x4E94914F0001LL) {
        Sleep(86400000);
        return;
    }
    ms = (DWORD)(delta / 1000000);
    if (1000000LL * (delta / 1000000) < delta)
        ms++;
    Sleep(ms);
}

void TimingSleepDeltaNs(long long deltaNs)
{
    long long now = QueryRaw();
    long long target;

    if (deltaNs >= 0x7FFFFFFFFFFFFFFFLL - now)
        target = 0x7FFFFFFFFFFFFFFFLL;
    else
        target = now + deltaNs;
    TimingSleepUntilNs(target);
}
