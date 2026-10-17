#ifndef VEXORE_TYPES_H
#define VEXORE_TYPES_H

typedef unsigned char byte;
typedef unsigned int dword;
typedef unsigned long long qword;
typedef int bool32;

struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
};

struct ColorRgba {
    float r;
    float g;
    float b;
    float a;
};

struct Color32 {
    byte r;
    byte g;
    byte b;
    byte a;
};

enum BoxEspType {
    BoxEsp2D = 0,
    BoxEsp2DFilled = 1,
    BoxEspCorner = 2,
    BoxEsp3D = 3,
    BoxEspOffscreen = 4
};

enum SkeletonType {
    SkeletonLines = 0,
    SkeletonDots = 1,
    SkeletonBoth = 2
};

enum AimKeyMode {
    AimKeyHold = 0,
    AimKeyToggle = 1
};

enum SkillCheckZoneMode {
    SkillCheckDefaultZones = 0,
    SkillCheckPerfectOnly = 1,
    SkillCheckWideGreat = 2
};

enum ConfigStatus {
    ConfigOk = 0,
    ConfigMissing = 1,
    ConfigCorrupted = 2,
    ConfigAccessDenied = 3,
    ConfigWriteFailed = 4
};

struct EspConfig {
    bool32 enabled;
    float lerpSmooth;
    float maxDistance;
    int boxType;
    bool32 boxOutline;
    bool32 boxFill;
    float boxFillAlpha;
    bool32 boxGradient;
    float boxThickness;
    float boxDistanceFade;
    bool32 showHealthBar;
    bool32 showRecoverBar;
    int skeletonType;
    bool32 skeletonDots;
    bool32 nameUppercase;
    bool32 nameShadow;
    float nameScale;
    float nicknameOffX;
    float nicknameOffY;
    float nameAngle;
    ColorRgba colBoxSurv;
    ColorRgba colBoxKiller;
    ColorRgba colBoxHealthy;
    ColorRgba colBoxInjured;
    ColorRgba colBoxDowned;
    ColorRgba colName;
    ColorRgba colHealthBar;
};

struct AimConfig {
    bool32 enabled;
    int key;
    int keyMode;
    float fovRadius;
    float smoothing;
    int bonePriority[4];
};

struct SkillCheckConfig {
    bool32 enabled;
    int zoneMode;
    float greatChance;
};

struct SpeedHackConfig {
    bool32 enabled;
    float value;
};

struct AuraToggle {
    bool32 enabled;
    float distance;
    ColorRgba color;
};

struct AuraConfig {
    bool32 masterSwitch;
    struct AuraToggle hatch;
    struct AuraToggle hooks;
    struct AuraToggle chests;
    struct AuraToggle totems;
    struct AuraToggle pallets;
    bool32 trapsMaster;
    float trapsDistance;
    bool32 trapBear;
    bool32 trapZombie;
    ColorRgba colorTrapBear;
    ColorRgba colorTrapZombie;
    float windowAuraMaxDist;
};

struct MenuTheme {
    float r;
    float g;
    float b;
    float alpha;
};

struct CheatConfig {
    struct EspConfig esp;
    struct AimConfig aim;
    struct SkillCheckConfig skillCheck;
    struct SpeedHackConfig speedHack;
    struct AuraConfig aura;
    struct MenuTheme theme;
};

struct GameStrings {
    char base[64];
    char window[64];
    char status[128];
};

#endif
