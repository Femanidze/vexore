#include "ApiResolvers.h"
#include "../core/Globals.h"

#include <windows.h>
#include <string.h>

int ApiResolversInit(struct ApiTable* t)
{
    if (!t)
        return 0;
    memset(t, 0, sizeof(*t));
    t->kernel32 = GetModuleHandleA("kernel32");
    if (!t->kernel32)
        t->kernel32 = LoadLibraryA("kernel32");
    if (!t->kernel32)
        return 0;
    t->getProc = (FnGetProc)GetProcAddress((HMODULE)t->kernel32, "GetProcAddress");
    t->loadLib = (FnLoadLib)GetProcAddress((HMODULE)t->kernel32, "LoadLibraryA");
    if (!t->getProc || !t->loadLib)
        return 0;
    t->sleepMs = (FnWinApi)t->getProc((HMODULE)t->kernel32, "Sleep");
    t->getTickCount = (FnWinApi)t->getProc((HMODULE)t->kernel32, "GetTickCount");
    t->user32 = (void*)GetModuleHandleA("user32");
    if (!t->user32 && t->loadLib)
        t->user32 = t->loadLib("user32");
    return 1;
}

void* ApiResolverSym(struct ApiTable* t, const char* mod, const char* name)
{
    void* m;
    if (!t || !t->getProc || !name)
        return NULL;
    if (mod && mod[0]) {
        m = (void*)GetModuleHandleA(mod);
        if (!m && t->loadLib)
            m = t->loadLib(mod);
        if (!m)
            return NULL;
        return (void*)t->getProc((HMODULE)m, name);
    }
    if (t->kernel32)
        return (void*)t->getProc((HMODULE)t->kernel32, name);
    return NULL;
}

void ApiResolversShutdown(struct ApiTable* t)
{
    if (!t)
        return;
    t->getProc = NULL;
    t->loadLib = NULL;
    t->sleepMs = NULL;
    t->getTickCount = NULL;
    t->user32 = NULL;
    t->kernel32 = NULL;
}
