#ifndef VEXORE_CONFIGLOAD_H
#define VEXORE_CONFIGLOAD_H

#include "../core/Types.h"

struct ConfigPathResult {
    int status;
    int fallbackUsed;
    char path[260];
};

int ConfigBuildDocumentPath(const char* fileName, struct ConfigPathResult* out);
int ConfigBuildFallbackPath(const char* fileName, struct ConfigPathResult* out);
int ConfigEnsureDirectory(const char* dirPath);
int ConfigLoadFromFile(const char* path, struct CheatConfig* cfg);
int ConfigSaveToFile(const char* path, const struct CheatConfig* cfg);
int ConfigDeleteFile(const char* path);
int ConfigRenameFile(const char* oldPath, const char* newPath);
int ConfigSelectExisting(const char* preferredDir, const char* fallbackDir, char* outPath, int outCap);

#endif
