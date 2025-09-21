#include "peregrine.h"
#include "../../infra/plugin_api.h"   // for ctx (PluginContext)
#include "../../AI/peregrine/peregrine.h"

extern PeregrineAPI peregrine_api;

// Global route state
static PeregrineAI g_route = {0};
static const PluginContext* g_ctx = nullptr;

// Forward declaration of timer callback
static int pgn_route_step_cb(int tid, uint64_t tick, int id, intptr_t data);

void pgn_route_start(const PluginContext* ctx,
                     struct map_session_data* sd,
                     PStepList* steps) {
    if (!sd || !steps || !ctx) return;

    g_ctx = ctx;  // store context globally

    g_route.steps = *steps;
    g_route.index = 0;
    g_route.sd = sd;
    g_route.active = true;

    // Schedule first step (200ms delay)
    g_ctx->timer->add_timer(g_ctx->timer->gettick() + 200,
                            pgn_route_step_cb, 0, 0);
}

void pgn_route_stop(void) {
    if (g_route.active) {
        g_ctx->peregrine->free_steps(&g_route.steps);  // note: PeregrineAPI call
        g_route.active = false;
        g_route.index = 0;
        g_route.sd = NULL;
    }
}

bool pgn_route_active(void) {
    return g_route.active;
}

// Timer callback: execute next step
static int pgn_route_step_cb(int tid, uint64_t tick, int id, intptr_t data) {
    if (!g_route.active || !g_route.sd) return 0;

    if (g_route.index >= g_route.steps.count) {
        pgn_route_stop();
        return 0;
    }

    int nx = g_route.steps.steps[g_route.index].x;
    int ny = g_route.steps.steps[g_route.index].y;

    g_ctx->movement->pc_walktoxy(g_route.sd, (short)nx, (short)ny, 0);
    g_route.index++;

    // Random delay between 200–400ms
    g_ctx->timer->add_timer(g_ctx->timer->gettick() + 200 + (rand() % 200),
                          pgn_route_step_cb, 0, 0);

    return 0;
}
