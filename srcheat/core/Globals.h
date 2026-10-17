#ifndef VEXORE_GLOBALS_H
#define VEXORE_GLOBALS_H

#include "Types.h"

#define VEX_CONFIG_DIR_DOCUMENTS "Vexore_Configs"
#define VEX_CONFIG_DIR_FALLBACK "C:\\Vexore_Configs"
#define VEX_CONFIG_EXT ".json"
#define VEX_CONFIG_TITLE_ACCESS "VEXORE Config Access Error"
#define VEX_CONFIG_TITLE_FATAL "VEXORE Fatal Error"
#define VEX_CONFIG_TITLE_SAVE "VEXORE Save Error"
#define VEX_CONFIG_TITLE_LOAD "VEXORE Load Error"
#define VEX_CONFIG_TITLE_WRITE_ACCESS "VEXORE Write Access Error"
#define VEX_CONFIG_TITLE_WRITE_FAILED "VEXORE Write Failed"
#define VEX_CONFIG_TITLE_CORRUPT "VEXORE Corrupted Config"
#define VEX_CONFIG_TITLE_DELETE "VEXORE Delete Error"
#define VEX_CONFIG_TITLE_RENAME "VEXORE Rename Error"
#define VEX_MSG_ACCESS_DENIED "Cannot access Documents folder (permission denied / blocked by OneDrive).\nDo you want to create config folder in C:\\Vexore_Configs instead?"
#define VEX_MSG_WRITE_DENIED "Cannot write file to current directory.\nSave to C:\\Vexore_Configs instead?"
#define VEX_MSG_FIRST_MATCH "First match detected! Game base permanently cemented."
#define VEX_MSG_UNSTABLE "Unstable game memory state. Retrying connection..."
#define VEX_MSG_CONNECTED "VEXORE.CC connected. Patched signatures successfully!"
#define VEX_MSG_LOBBY_KILLER "Lobby Killer: "
#define VEX_WIN_CLASS_GAME "DeadByDaylight  "
#define VEX_WIN_TITLE_GAME "UnrealWindow"

extern volatile byte g_runningFlag;
extern volatile dword g_moduleHandle;
extern volatile qword g_gameBase;
extern volatile qword g_signatureOffA;
extern volatile qword g_signatureOffB;
extern volatile byte g_baseCemented;
extern volatile byte g_connected;
extern dword g_espWindowWidth;
extern dword g_espWindowHeight;

extern struct CheatConfig g_config;

#endif
