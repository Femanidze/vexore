#ifndef VEXORE_GAMERUNTIME_H
#define VEXORE_GAMERUNTIME_H

#include "../core/Types.h"

enum EntClass {
    EntNone = 0,
    EntSurvivor = 1,
    EntGenerator = 2,
    EntHatch = 3,
    EntTotem = 4,
    EntPallet = 5,
    EntChest = 6,
    EntHook = 7,
    EntWindow = 8,
    EntTrap = 9,
    EntExit = 10
};

struct EspTargetState {
    struct Vec3 world;
    struct Vec2 screen;
    struct Vec2 screenLerp;
    int visible;
    int healthState;
    float distance;
    char name[64];
    int entityType;
    float progressPct;
    float recoveryPct;
    qword actorAddr;
    qword entityAddr;
};

struct GameRuntimeState {
    int featureFrame;
    int signatureFrame;
    dword targetWidth;
    dword targetHeight;
    qword cachedBase;
    struct EspTargetState targets[64];
    int targetCount;
    struct Vec2 aimDelta;
    int aimActive;
    int worldValid;
    qword rootAddr;
    qword midAddr;
    qword entityArray;
    unsigned int entityCount;
    qword playerArray;
    unsigned int playerCount;
    qword localActor;
    struct Vec3 localPos;
    struct Vec3 aimTargetWorld;
    int aimTargetValid;
    int aimOnTarget;
};

extern struct GameRuntimeState g_runtime;

void GameFeatureTick(void);
void SignatureScanTick(unsigned int moduleHandle, unsigned int width);
int GameRuntimeProject(const struct Vec3* world, struct Vec2* out);
void GameRuntimeLerpTargets(void);
void GameRuntimeSetTargets(const struct EspTargetState* list, int count);
int GameSafeRead(qword addr, void* out, unsigned int size);
int GameSafeWrite(qword addr, const void* in, unsigned int size);
int IsValidPtr(qword addr);
qword GameResolveRoot(void);
qword GameResolveEntityArray(unsigned int* outCount);
qword GameResolvePlayerArray(unsigned int* outCount);
int GameReadEntityRecord(qword entity, struct EspTargetState* out);
int GameClassifyName(const char* name);
int GameReadActorPosition(qword actor, struct Vec3* out);
int GameReadActorHealth(qword actor, float* health, float* progress, float* recovery);

#endif
