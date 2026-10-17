#ifndef VEXORE_APIRESOLVERS_H
#define VEXORE_APIRESOLVERS_H

#include "../core/Types.h"

typedef void* (__stdcall* FnGetProc)(void* module, const char* name);
typedef void* (__stdcall* FnLoadLib)(const char* name);
typedef unsigned long (__stdcall* FnWinApi)(void);

struct ApiTable {
    FnGetProc getProc;
    FnLoadLib loadLib;
    FnWinApi sleepMs;
    FnWinApi getTickCount;
    void* user32;
    void* kernel32;
};

int ApiResolversInit(struct ApiTable* t);
void* ApiResolverSym(struct ApiTable* t, const char* mod, const char* name);
void ApiResolversShutdown(struct ApiTable* t);

#endif
