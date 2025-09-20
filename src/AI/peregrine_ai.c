// peregrine_ai.c
// FalconPM - High-level path executor (like OpenKore AI)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "peregrine_ai.h"
#include "peregrine_gat.h"
#include "peregrine_path.h"
#include "../infra/plugin_api.h"   // for UnitAPI, TimerAPI, etc.

extern const SmartAPI FPM_SmartAPI;   // exposed by fpm_path.c
extern const UnitAPI* fpm_unit;       // injected from FalconPM base
extern const TimerAPI* fpm_timer;     // injected for scheduling

// jitter helper
static int jitter_ms(int base) {
    int delta = rand() % 200; // 0–199ms
    return base + delta;
}

bool peregrineAI(struct map_session_data* sd, const char* mapname, int tx, int ty) {
    if (!sd || !mapname) return false;

    char pathbuf[256];
    snprintf(pathbuf, sizeof(pathbuf), "db/gat/%s.gat", mapname);

    GatMap* g = FPM_SmartAPI.load_gat(pathbuf);
    if (!g) {
        fprintf(stderr, "[peregrineAI] failed to load map %s\n", mapname);
        return false;
    }

    int sx = sd->bl.x;
    int sy = sd->bl.y;

    printf("[peregrineAI] %s (%d,%d) -> (%d,%d)\n",
           mapname, sx, sy, tx, ty);

    PStepList steps;
    if (!FPM_SmartAPI.astar(g, sx, sy, tx, ty, &steps)) {
        fprintf(stderr, "[peregrineAI] no path found\n");
        FPM_SmartAPI.free_gat(g);
        return false;
    }

    // Schedule step execution
    for (int i=0; i<steps.count; i++) {
        int x = steps.steps[i].x;
        int y = steps.steps[i].y;

        // wrap into a lambda/closure — here simplified
        // (in practice you'd queue into timer system)
        fpm_timer->add(sd, jitter_ms(150), (TimerCallback)fpm_unit->walktoxy, x, y);
    }

    FPM_SmartAPI.free_steps(&steps);
    FPM_SmartAPI.free_gat(g);
    return true;
}
