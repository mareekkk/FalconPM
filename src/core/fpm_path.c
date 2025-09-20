/**
 * FalconPM - Pathfinding
 * fpm_path.c
 * Local pathfinding inside one map (x,y steps)
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>          // for usleep (future humanization)
#include "../infra/plugin_api.h"  // bring in PluginContext + APIs

// Step structure
typedef struct {
    int x, y;
} FPM_Step;

#define MAX_STEPS 512

typedef struct {
    FPM_Step steps[MAX_STEPS];
    int count;
} FPM_StepList;

// ------------------------------------------------------------
// Simple straight-line pathfinder (stub)
// ------------------------------------------------------------
bool fpm_pathfind(int x1, int y1, int x2, int y2, FPM_StepList *out) {
    if (!out) return false;
    out->count = 0;

    int dx = (x2 > x1) ? 1 : -1;
    int dy = (y2 > y1) ? 1 : -1;

    int x = x1, y = y1;
    while (x != x2 || y != y2) {
        if (x != x2) x += dx;
        if (y != y2) y += dy;

        if (out->count < MAX_STEPS) {
            out->steps[out->count].x = x;
            out->steps[out->count].y = y;
            out->count++;
        } else {
            return false; // path too long
        }
    }
    return true;
}

// ------------------------------------------------------------
// FalconPM-aware executor: walk along path
// ------------------------------------------------------------
bool fpm_path_execute(struct map_session_data* sd,
                      int x1, int y1, int x2, int y2,
                      const PluginContext* ctx) {
    if (!sd || !ctx || !ctx->movement) return false;

    FPM_StepList steps;
    if (!fpm_pathfind(x1, y1, x2, y2, &steps)) {
        if (ctx->log) ctx->log->error("[fpm_path] failed to pathfind");
        return false;
    }

    if (ctx->log) ctx->log->info("[fpm_path] %d steps planned", steps.count);

    for (int i = 0; i < steps.count; i++) {
        ctx->movement->pc_walktoxy(sd,
                                   (short)steps.steps[i].x,
                                   (short)steps.steps[i].y,
                                   0);

        // ⚠️ Humanization placeholder:
        // usleep(100000 + (rand() % 200000)); // 100–300 ms pause
    }
    return true;
}
