// src/plugins/autoroute/autoroute.cpp
// FalconPM Autoroute Plugin
// Provides @ar on/off for continuous roaming, or @ar <x> <y> for single walk

#include "../../infra/plugin_api.h"
#include "../../AI/peregrine/peregrine.h"
#include "../../AI/peregrine/pgn_path.h"
#include "../../core/falconpm.hpp"  
#include <cstdio>
#include <cstring>
#include <cstdlib>

// ----------------------------------------------------
// Plugin state
// ----------------------------------------------------
static const PluginContext* ctx = nullptr;
static bool roaming_enabled = false;
static map_session_data* roaming_sd = nullptr;

// ----------------------------------------------------
// Helper: pick a random walkable coordinate
// ----------------------------------------------------
static bool pick_random_walkable(GatMap* g, int& x, int& y) {
    for (int tries = 0; tries < 100; ++tries) {
        int rx = rand() % g->width;
        int ry = rand() % g->height;
        if (ctx->peregrine->is_walkable(g, rx, ry)) {
            x = rx;
            y = ry;
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------
// Core autoroute logic (one shot to (x,y))
// ----------------------------------------------------
static void autoroute_to(map_session_data* sd, GatMap* g, int sx, int sy, int x, int y) {
    PStepList steps;
    bool ok = ctx->peregrine->astar(g, sx, sy, x, y, &steps);
    if (!ok || steps.count <= 0) {
        ctx->player->send_message(sd, "[Autoroute] No path found.");
        return;
    }

    pgn_route_start(ctx, sd, &steps, g);

    char buf[128];
    snprintf(buf, sizeof(buf), "[Autoroute] Path started to (%d,%d)", x, y);
    ctx->player->send_message(sd, buf);
}

// ----------------------------------------------------
// Roaming timer callback
// ----------------------------------------------------
static int roaming_timer_cb(int tid, uint64_t tick, int id, intptr_t data) {
    if (roaming_enabled && roaming_sd) {
        int m  = fpm_get_sd_m(roaming_sd);
        int sx = fpm_get_sd_x(roaming_sd);
        int sy = fpm_get_sd_y(roaming_sd);

        const char* mapname = fpm_get_map_name(m);
        if (!mapname || mapname[0] == '\0') return 0;

        char filename[512];
        snprintf(filename, sizeof(filename), FALCONPM_GAT_PATH "%s.gat", mapname);

        GatMap* g = ctx->peregrine->load_gat(filename);
        if (!g) return 0;

        int x = 0, y = 0;
        if (pick_random_walkable(g, x, y)) {
            ctx->log->info("[autoroute] Roaming target chosen (%d,%d)", x, y);
            autoroute_to(roaming_sd, g, sx, sy, x, y);
        }

        // schedule again after 5 seconds
        ctx->timer->add_timer(ctx->timer->gettick() + 5000,
                              roaming_timer_cb, 0, 0);
    }
    return 0;
}

// ----------------------------------------------------
// Command implementation
// ----------------------------------------------------
static int cmd_autoroute(map_session_data* sd, const char* cmd, const char* msg) {
    if (!sd) return -1;

    // Continuous roaming ON
    if (msg && strcmp(msg, "on") == 0) {
        roaming_enabled = true;
        roaming_sd = sd;
        ctx->player->send_message(sd, "[Autoroute] Continuous roaming ENABLED");
        ctx->timer->add_timer(ctx->timer->gettick() + 1000,
                              roaming_timer_cb, 0, 0);
        return 0;
    }

    // Continuous roaming OFF
    if (msg && strcmp(msg, "off") == 0) {
        roaming_enabled = false;
        roaming_sd = nullptr;
        ctx->player->send_message(sd, "[Autoroute] Continuous roaming DISABLED");
        return 0;
    }

    // ----------------------------------------------------
    // Otherwise: single walk (random or explicit coords)
    // ----------------------------------------------------
    int m  = fpm_get_sd_m(sd);
    int sx = fpm_get_sd_x(sd);
    int sy = fpm_get_sd_y(sd);

    const char* mapname = fpm_get_map_name(m);
    if (!mapname || mapname[0] == '\0') {
        ctx->log->error("[autoroute] Invalid map index %d", m);
        return -1;
    }

    char filename[512];
    snprintf(filename, sizeof(filename), FALCONPM_GAT_PATH "%s.gat", mapname);

    GatMap* g = ctx->peregrine->load_gat(filename);
    if (!g) {
        ctx->log->error("[autoroute] Failed to load GAT for %s", mapname);
        return -1;
    }

    int x = 0, y = 0;
    if (!msg || std::strlen(msg) == 0) {
        // default random
        if (!pick_random_walkable(g, x, y)) {
            ctx->player->send_message(sd, "[Autoroute] Failed to find valid roam target.");
            return -1;
        }
    } else {
        if (sscanf(msg, "%d %d", &x, &y) != 2) {
            ctx->player->send_message(sd, "[Autoroute] Invalid coordinates.");
            return -1;
        }
        if (!ctx->peregrine->is_walkable(g, x, y)) {
            ctx->player->send_message(sd, "[Autoroute] Target cell is not walkable.");
            return -1;
        }
    }

    autoroute_to(sd, g, sx, sy, x, y);
    return 0;
}

// ----------------------------------------------------
// Plugin init/final
// ----------------------------------------------------
extern "C" {

bool plugin_init(const PluginContext* unused) {
    (void)unused;  // loader context not used
    ctx = falconpm_get_context();   // ✅ always use global context
    ctx->atcommand->add("ar", cmd_autoroute);
    ctx->log->info("[autoroute] Plugin initialized (@ar on/off)");
    return true;
}

void plugin_final() {
    roaming_enabled = false;
    roaming_sd = nullptr;
    ctx->log->info("[autoroute] Plugin shutting down.");
}

PluginDescriptor PLUGIN = {
    "autoroute",
    "0.2",
    nullptr,
    plugin_init,
    plugin_final
};

} // extern "C"
