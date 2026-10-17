#ifndef VEXORE_CONFIGIO_H
#define VEXORE_CONFIGIO_H

#include "../core/Types.h"
#include "ConfigLoad.h"

int ConfigIoPickDirectory(char* outDir, int cap);
int ConfigIoLoad(const char* path, struct CheatConfig* cfg);
int ConfigIoSave(const char* path, const struct CheatConfig* cfg);
const char* ConfigIoDefaultName(void);

#endif
