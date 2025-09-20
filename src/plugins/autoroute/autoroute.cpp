// autoroute.cpp
// FalconPM Autoroute plugin
// Features:
//   - @ar on/off → continuous random roaming
//   - @ar x y    → walk to given coordinates
//   - @ar        → single random walk
//
// Uses FalconPM PathAPI + DirectionAPI to walk full-map routes.

#include "../../infra/plugin_api.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>   // for usleep()

// rAthena headers
#include "map.hpp"
#include "pc.hpp"
#include "unit.hpp"
#include "path.hpp"   // walkpath_data

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
// Pick a random destination on the map and walk there
// ----------------------------------------------------
static void autoroute_random(map_session_data* sd) {
    if (!sd || !ctx || !ctx->rnd) return;

    int sx = sd->x;
    int sy = sd->y;

    int maxx = map[sd->m].xs;
    int maxy = map[sd->m].ys;

    // Random destination + jitter
    int tx = ctx->rnd->rnd() % maxx;
    int ty = ctx->rnd->rnd() % maxy;
    tx += (rand() % 5) - 2;
    ty += (rand() % 5) - 2;

    if (tx < 0) tx = 0;
    if (ty < 0) ty = 0;
    if (tx >= maxx) tx = maxx - 1;
    if (ty >= maxy) ty = maxy - 1;

    ctx->log->info("[autoroute] target (%d,%d) on %s",
                   tx, ty, map[sd->m].name);

    // --- Path search
    struct walkpath_data wpd;
    int ok = ctx->path->path_search(&wpd, sd->m, sx, sy, tx, ty, 0);
    if (!ok || wpd.path_len <= 0) {
        ctx->log->error("[autoroute] no path found to (%d,%d)", tx, ty);
        return;
    }

    // --- Follow path step by step
    for (int i = 0; i < wpd.path_len; i++) {
        int dir = wpd.path[i];
        int nx = sx + ctx->dir->dx[dir];
        int ny = sy + ctx->dir->dy[dir];

        ctx->movement->pc_walktoxy(sd, (short)nx, (short)ny, 0);
        sx = nx;
        sy = ny;

        // Humanization: random delay 100–300ms
        usleep(100000 + (rand() % 200000));

        // Pause longer every ~10 steps
        if (i > 0 && i % 10 == 0) {
            usleep(500000 + (rand() % 500000)); // 0.5–1s
        }
    }
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

    int x, y;
    if (sscanf(args, "%d %d", &x, &y) < 2) {
        // No coords → do a single random walk
        autoroute_random(sd);
        return 0;
    }

    ctx->log->info("[autoroute] walking to (%d,%d)", x, y);

    // Same path logic as autoroute_random, but fixed target
    struct walkpath_data wpd;
    int sx = sd->x, sy = sd->y;
    int ok = ctx->path->path_search(&wpd, sd->m, sx, sy, x, y, 0);
    if (!ok || wpd.path_len <= 0) {
        ctx->log->error("[autoroute] no path found to (%d,%d)", x, y);
        return 0;
    }

    for (int i = 0; i < wpd.path_len; i++) {
        int dir = wpd.path[i];
        int nx = sx + ctx->dir->dx[dir];
        int ny = sy + ctx->dir->dy[dir];

        ctx->movement->pc_walktoxy(sd, (short)nx, (short)ny, 0);
        sx = nx; sy = ny;
        usleep(100000 + (rand() % 200000));
    }

    return 0;
}

// ----------------------------------------------------
// Plugin boilerplate
// ----------------------------------------------------
static bool init(const PluginContext* c) {
    ctx = falconpm_get_context();
    if (!ctx || !ctx->atcommand || !ctx->path || !ctx->dir) {
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
