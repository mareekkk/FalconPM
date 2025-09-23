// src/plugins/autoroute/autoroute.cpp
// FalconPM Autoroute Plugin
// Provides @ar to test Peregrine pathfinding

#include "../../infra/plugin_api.h"
#include "../../AI/peregrine/peregrine.h"
#include "../../AI/peregrine/pgn_path.h"
#include "../../core/falconpm.hpp"  
#include <cstdio>
#include <cstring>

// ----------------------------------------------------
// Plugin state
// ----------------------------------------------------
static const PluginContext* ctx = nullptr;

// ----------------------------------------------------
// Command implementation
// ----------------------------------------------------
static int cmd_autoroute(map_session_data* sd, const char* cmd, const char* msg) {
    if (!sd) return -1;

    if (!msg || std::strlen(msg) == 0) {
        ctx->player->send_message(sd, "[Autoroute] Usage: @ar <x> <y>");
        return -1;
    }

    int x = 0, y = 0;
    if (sscanf(msg, "%d %d", &x, &y) != 2) {
        ctx->player->send_message(sd, "[Autoroute] Invalid coordinates.");
        return -1;
    }

    // ----------------------------------------------------
    // Get map and player position via bootstrap wrappers
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

    // ----------------------------------------------------
    // Run A* pathfinding
    // ----------------------------------------------------
    PStepList steps;
    bool ok = ctx->peregrine->astar(g, sx, sy, x, y, &steps);
    if (!ok || steps.count <= 0) {
        ctx->player->send_message(sd, "[Autoroute] No path found.");
        return -1;
    }

    ctx->player->send_message(sd, "[Autoroute] Path calculated.");
    ctx->peregrine->free_steps(&steps);
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
    ctx->log->info("[autoroute] Plugin initialized (@ar)");
    return true;
}

void plugin_final() {
    ctx->log->info("[autoroute] Plugin shutting down.");
}

PluginDescriptor PLUGIN = {
    "autoroute",
    "0.1",
    nullptr,
    plugin_init,
    plugin_final
};

} // extern "C"
