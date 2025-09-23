#include "peregrine.h"
#include <cstdlib>

static const PluginContext* g_ctx = nullptr;
PeregrineAI g_route = {0};

bool pgn_route_active(void) {
    return g_route.active;
}

// ----------------------------------------------------
// Step callback
// ----------------------------------------------------
static int pgn_route_step_cb(int tid, uint64_t tick, int id, intptr_t data) {
    if (!g_route.active || !g_route.sd) return 0;
    if (g_route.index >= g_route.steps.count) {
        g_route.active = false;
        return 0;
    }

    int nx = g_route.steps.steps[g_route.index].x;
    int ny = g_route.steps.steps[g_route.index].y;

    // -- micro jitter (±1 cell)
    int jx = nx + ((rand() % 3) - 1);
    int jy = ny + ((rand() % 3) - 1);
    if (jx < 0) jx = 0;
    if (jy < 0) jy = 0;

    // -- check walkability
    if (g_ctx->peregrine->is_walkable(g_route.gmap, jx, jy)) {
        g_ctx->movement->pc_walktoxy(g_route.sd, (short)jx, (short)jy, 0);
    } else if (g_ctx->peregrine->is_walkable(g_route.gmap, nx, ny)) {
        g_ctx->movement->pc_walktoxy(g_route.sd, (short)nx, (short)ny, 0);
    } else {
        g_ctx->log->info("[humanize] blocked cell (%d,%d), skipping", nx, ny);
    }

    g_route.index++;

    // -- variable delay (150–350ms)
    int delay = 150 + (rand() % 200);
    g_ctx->timer->add_timer(g_ctx->timer->gettick() + delay,
                            pgn_route_step_cb, 0, 0);

    return 0;
}

// ----------------------------------------------------
// Start route
// ----------------------------------------------------
void pgn_route_start(const PluginContext* ctx,
                     struct map_session_data* sd,
                     PStepList* steps,
                     GatMap* gmap) {
    if (!sd || !steps || !ctx || !gmap) return;

    g_ctx = ctx;
    g_route.steps = *steps;
    g_route.index = 0;
    g_route.sd = sd;
    g_route.active = true;
    g_route.gmap = gmap;

    g_ctx->timer->add_timer(g_ctx->timer->gettick() + 200,
                            pgn_route_step_cb, 0, 0);
}

// ----------------------------------------------------
// Stop route
// ----------------------------------------------------
void pgn_route_stop() {
    if (!g_ctx) return;
    g_route.active = false;
    if (g_route.steps.steps) {
        g_ctx->peregrine->free_steps(&g_route.steps);
    }
}
