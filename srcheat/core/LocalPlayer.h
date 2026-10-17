#ifndef VEXORE_LOCALPLAYER_H
#define VEXORE_LOCALPLAYER_H

#include "../core/Types.h"

struct LocalPlayerState {
    qword world;
    qword mid;
    qword entityList;
    qword localActor;
    struct Vec3 position;
    float healthPct;
    int valid;
};

extern struct LocalPlayerState g_localPlayer;

int LocalPlayerResolve(void);
int LocalPlayerReadPosition(struct Vec3* out);
float LocalPlayerReadHealthPct(void);
qword LocalPlayerActor(void);

#endif
