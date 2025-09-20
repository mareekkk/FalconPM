// /home/marek/FalconPM/src/plugins/autoroute/autoroute.cpp
//
// FalconPM Autoroute plugin
// Provides @ar <x> <y> command to walk the player to a target cell.
// Extended: if no args, chooses a random cell on the current map.
//

#include "../../infra/plugin_api.h"   // FalconPM API
#include <cstdio>
#include <cstdlib>
#include <cstring>

// rAthena headers (modern C++ .hpp style)
#include "map.hpp"     // defines map_session_data and map_data (xs, ys, name)
#include "pc.hpp"      // player character functions
#include "unit.hpp"    // unit movement helpers

extern struct map_data map[];   // declared in map.hpp

// ----------------------------------------------------
// Access FalconPM context from base
// ----------------------------------------------------
extern "C" const PluginContext* falconpm_get_context(void);
static const PluginContext* ctx = nullptr;

// Forward declare path executor (from fpm_path.c)
extern "C" bool fpm_path_execute(struct map_session_data* sd,
                                 int x1, int y1, int x2, int y2,
                                 const PluginContext* ctx);

// ----------------------------------------------------
// Helper: choose random destination
// ----------------------------------------------------
static void autoroute_random(map_session_data* sd) {
    if (!sd || !ctx || !ctx->rnd) return;

    int sx = sd->x;
    int sy = sd->y;

    // Map bounds from global map[]
    int maxx = map[sd->m].xs;
    int maxy = map[sd->m].ys;

    int tx = ctx->rnd->rnd() % maxx;
    int ty = ctx->rnd->rnd() % maxy;

    ctx->log->info("[autoroute] random target (%d,%d) on map %s",
                   tx, ty, map[sd->m].name);

    if (!fpm_path_execute(sd, sx, sy, tx, ty, ctx)) {
        ctx->log->error("[autoroute] failed to walk random path");
    }
}

// ----------------------------------------------------
// Command handler for @ar
// ----------------------------------------------------
static int at_ar(map_session_data* sd, const char* cmd, const char* args) {
    if (!sd) return -1;

    int x, y;
    if (sscanf(args, "%d %d", &x, &y) < 2) {
        // No args → random mode
        autoroute_random(sd);
        return 0;
    }

    // Explicit target
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
    fprintf(stdout, "[autoroute] init OK\n");
    return true;
}

static void shutdown(void) {
    fprintf(stdout, "[autoroute] shutdown\n");
}

extern "C" {
PluginDescriptor PLUGIN = {
    "autoroute",
    "0.7",
    nullptr,   // required modules
    init,
    shutdown
};
}
