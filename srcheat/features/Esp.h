#ifndef VEXORE_ESP_H
#define VEXORE_ESP_H

#include "../core/Types.h"
#include "GameRuntime.h"

void EspOverlayApply(const struct EspConfig* cfg);
void EspResolveNames(struct EspConfig* cfg);
float EspComputeFade(const struct EspConfig* cfg, float distance);
int EspProjectTarget(const struct Vec3* world, struct Vec2* out);
void EspUppercaseName(char* name);

#endif
