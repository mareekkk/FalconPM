// /home/marek/FalconPM/src/plugins/autoroute/autoroute.cpp
//
// FalconPM Autoroute plugin
// Extended: @ar on/off to enable continuous random roaming.

#include "../../infra/plugin_api.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

// rAthena headers
#include "map.hpp"
#include "pc.hpp"
#include "unit.hpp"

// ----------------------------------------------------
// Access FalconPM context
// ----------------------------------------------------
extern "C" const PluginContext* falconpm_get_context(void);
static const PluginContext* ctx = nullptr;

// Forward declare path executor
extern "C" bool fpm_path_execute(struct map_session_data* sd,
                                 int x1, int y1, int x2, int y2,
                                 const PluginContext* ctx);

// ----------------------------------------------------
// Roaming state
// ----------------------------------------------------
static bool roaming_enabled = false;
static map_session_data* roaming_sd = nullptr;

// ----------------------------------------------------
// Helper: choose random destination (with jitter)
// ----------------------------------------------------
static void autoroute_random(map_session_data* sd) {
    if (!sd || !ctx || !ctx->rnd) return;

    int sx = sd->x;
    int sy = sd->y;

    int maxx = map[sd->m].xs;
    int maxy = map[sd->m].ys;

    int tx = ctx->rnd->rnd() % maxx;
    int ty = ctx->rnd->rnd() % maxy;

    // --- Humanization: jitter target ±1–2 tiles ---
    int jitter_x = (rand() % 5) - 2; // -2 .. +2
    int jitter_y = (rand() % 5) - 2; // -2 .. +2

    tx += jitter_x;
    ty += jitter_y;

    // Clamp inside map bounds
    if (tx < 0) tx = 0;
    if (ty < 0) ty = 0;
    if (tx >= maxx) tx = maxx - 1;
    if (ty >= maxy) ty = maxy - 1;

    ctx->log->info("[autoroute] random target (%d,%d) on map %s (jitter %d,%d)",
                   tx, ty, map[sd->m].name, jitter_x, jitter_y);

    if (!fpm_path_execute(sd, sx, sy, tx, ty, ctx)) {
        ctx->log->error("[autoroute] failed to walk random path");
    }
}

// ----------------------------------------------------
// Timer callback (fires every 5s if roaming is enabled)
// ----------------------------------------------------
static int roaming_timer_cb(int tid, uint64_t tick, int id, intptr_t data) {
    if (roaming_enabled && roaming_sd) {
        autoroute_random(roaming_sd);
        // reschedule after 5s
        ctx->timer->add_timer(ctx->timer->gettick() + 5000,
                              roaming_timer_cb, 0, 0);
    }
    return 0;
}

// ----------------------------------------------------
// Command handler for @ar
// ----------------------------------------------------
static int at_ar(map_session_data* sd, const char* cmd, const char* args) {
    if (!sd) return -1;

    if (strcmp(args, "on") == 0) {
        roaming_enabled = true;
        roaming_sd = sd;
        ctx->log->info("[autoroute] roaming enabled");
        ctx->timer->add_timer(ctx->timer->gettick() + 5000,
                              roaming_timer_cb, 0, 0);
        return 0;
    }

    if (strcmp(args, "off") == 0) {
        roaming_enabled = false;
        roaming_sd = nullptr;
        ctx->log->info("[autoroute] roaming disabled");
        return 0;
    }

    int x, y;
    if (sscanf(args, "%d %d", &x, &y) < 2) {
        // No coords, single random walk
        autoroute_random(sd);
        return 0;
    }

    ctx->log->info("[autoroute] @ar walking to (%d,%d)", x, y);

    if (!fpm_path_execute(sd, sd->x, sd->y, x, y, ctx)) {
        ctx->log->error("[autoroute] failed to walk explicit path");
    }
    return 0;
}

// ----------------------------------------------------
// FalconPM plugin boilerplate
// ----------------------------------------------------
static bool init(const PluginContext* c) {
    ctx = falconpm_get_context();
    if (!ctx || !ctx->atcommand) {
        fprintf(stderr, "[autoroute] missing AtcommandAPI\n");
        return false;
    }

    ctx->atcommand->add("ar", at_ar);

    fprintf(stdout, "[autoroute] init OK (use @ar on/off)\n");
    return true;
}

static void shutdown(void) {
    roaming_enabled = false;
    roaming_sd = nullptr;
    fprintf(stdout, "[autoroute] shutdown\n");
}

extern "C" {
PluginDescriptor PLUGIN = {
    "autoroute",
    "0.9",
    nullptr,
    init,
    shutdown
};
}


