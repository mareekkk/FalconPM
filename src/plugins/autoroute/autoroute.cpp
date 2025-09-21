// autoroute.cpp
// FalconPM Autoroute plugin
// Features:
//   - @ar on/off → continuous random roaming
//   - @ar        → single random walk
//
// Uses FalconPM PeregrineAPI (Peregrine) for GAT-based routing.

#include "../../infra/plugin_api.h"
#include "../../AI/peregrine/pgn_path.h"   // PeregrineAPI step list
#include "../../AI/peregrine/peregrine.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// rAthena session structures
#include "map.hpp"
#include "pc.hpp"

// ----------------------------------------------------
// FalconPM context
// ----------------------------------------------------
extern "C" const PluginContext* falconpm_get_context(void);
static const PluginContext* ctx = nullptr;

// ----------------------------------------------------
// Roaming state
// ----------------------------------------------------
static bool roaming_enabled = false;
static map_session_data* roaming_sd = nullptr;

// ----------------------------------------------------
// PeregrineAI: pick a random destination and walk there
// ----------------------------------------------------
static void autoroute_random(map_session_data* sd) {
    if (!sd || !ctx || !ctx->rnd || !ctx->peregrine) return;

    int sx = sd->x;
    int sy = sd->y;
    int maxx = map[sd->m].xs;
    int maxy = map[sd->m].ys;

    // --- Load GAT map (cache per map)
    static GatMap* g = nullptr;
    static std::string gmap_name;
    if (!g || gmap_name != map[sd->m].name) {
        if (g) ctx->peregrine->free_gat(g);
        char filename[512];
        snprintf(filename, sizeof(filename),
         FALCONPM_GAT_PATH "%s.gat", map[sd->m].name);
g = ctx->peregrine->load_gat(filename);
        gmap_name = map[sd->m].name;
        if (!g) {
            ctx->log->error("[autoroute] failed to load GAT for %s", map[sd->m].name);
            return;
        }
    }

        // Pick a random walkable destination (retry up to 10 times)
    int tx = 0, ty = 0, tries = 0;
    do {
        tx = ctx->rnd->rnd() % maxx;
        ty = ctx->rnd->rnd() % maxy;
        tx += (rand() % 5) - 2;
        ty += (rand() % 5) - 2;

        if (tx < 0) tx = 0;
        if (ty < 0) ty = 0;
        if (tx >= maxx) tx = maxx - 1;
        if (ty >= maxy) ty = maxy - 1;

        tries++;
    } while (tries < 10 && !ctx->peregrine->is_walkable(g, tx, ty));

    ctx->log->info("[autoroute] player at (%d,%d) → target (%d,%d) on %s",
                sx, sy, tx, ty, map[sd->m].name);

                
    // --- Path search with PeregrineAPI
    PStepList steps;
    bool ok = ctx->peregrine->astar(g, sx, sy, tx, ty, &steps);
    if (!ok || steps.count <= 0) {
        ctx->log->error("[autoroute] no path found to (%d,%d)", tx, ty);
        return;
    }

    // old: manual walking loop
    // new: hand route to Peregrine
    pgn_route_start(ctx, sd, &steps, g);


    }

// ----------------------------------------------------
// Timer callback for roaming
// ----------------------------------------------------
static int roaming_timer_cb(int tid, uint64_t tick, int id, intptr_t data) {
    if (roaming_enabled && roaming_sd) {
        autoroute_random(roaming_sd);

        // schedule next tick after 5s
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

    // Default: single random walk
    autoroute_random(sd);
    return 0;
}

// ----------------------------------------------------
// Plugin boilerplate
// ----------------------------------------------------
static bool init(const PluginContext* c) {
    ctx = falconpm_get_context();
    if (!ctx || !ctx->atcommand || !ctx->peregrine || !ctx->movement || !ctx->timer) {
        fprintf(stderr, "[autoroute] missing APIs\n");
        return false;
    }
    ctx->atcommand->add("ar", at_ar);
    fprintf(stdout, "[autoroute] init OK (@ar on/off)\n");
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
    "1.0",
    nullptr,
    init,
    shutdown
};
}
