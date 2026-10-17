#ifndef VEXORE_OVERLAYRENDER_H
#define VEXORE_OVERLAYRENDER_H

#include "../core/Types.h"

void OverlayBeginFrame(void);
void OverlayDrawEsp(const struct EspConfig* cfg);
void OverlayDrawAura(const struct AuraConfig* cfg);
void OverlayDrawCrosshair(const struct CheatConfig* cfg);
void OverlayDrawWatermark(const struct CheatConfig* cfg);
void OverlayEndFrame(void);
void OverlayRenderAll(const struct CheatConfig* cfg);

#endif
