// src/plugins/autoattack/autoattack.cpp
// FalconPM AutoAttack Plugin
// State machine: SEARCH → ATTACK
// Uses Peregrine for movement, unit_attack for combat

#include "../../infra/plugin_api.h"
#include "../../AI/peregrine/peregrine.h"
#include "../../AI/peregrine/pgn_path.h"
#include "../../core/falconpm.hpp"  
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>

// ----------------------------------------------------
// Shim functions (from falconpm_bootstrap.cpp)
// ----------------------------------------------------
extern "C" {
    int fpm_get_sd_x(map_session_data* sd);
    int fpm_get_sd_y(map_session_data* sd);
    int fpm_get_sd_m(map_session_data* sd);
    int fpm_get_bl_x(block_list* bl);
    int fpm_get_bl_y(block_list* bl);
    int fpm_get_bl_id(block_list* bl);
    int fpm_get_map_width(int m);
    int fpm_get_map_height(int m);
    const char* fpm_get_map_name(int m);
}

// ----------------------------------------------------
// Plugin state
// ----------------------------------------------------
static const PluginContext* ctx = nullptr;  // ✅ single pointer to FalconPM

static bool g_enabled = false;
static int g_account_id = -1;
static block_list* g_current_target = nullptr;
static GatMap* g_gat_map = nullptr;
static uint64_t g_last_attack = 0;

// States: only SEARCHING + ATTACKING
enum AttackState {
    STATE_SEARCHING,
    STATE_ATTACKING
};
static AttackState g_state = STATE_SEARCHING;

// ----------------------------------------------------
// Load GAT map (cache per map)
// ----------------------------------------------------
static GatMap* load_gat_for_sd(map_session_data* sd) {
    if (!sd || !ctx || !ctx->peregrine) return nullptr;

    static GatMap* g = nullptr;
    static std::string gmap_name;

    int map_index = fpm_get_sd_m(sd);
    const char* mapname = fpm_get_map_name(map_index);
    if (!mapname || mapname[0] == '\0') return nullptr;

    if (!g || gmap_name != std::string(mapname)) {
        if (g) {
            ctx->peregrine->free_gat(g);
            g = nullptr;
        }

        char filename[512];
        snprintf(filename, sizeof(filename), FALCONPM_GAT_PATH "%s.gat", mapname);

        g = ctx->peregrine->load_gat(filename);
        gmap_name = mapname;

        if (!g) {
            ctx->log->error("[autoattack] Failed to load GAT for %s (%s)", mapname, filename);
            return nullptr;
        }

        ctx->log->info("[autoattack] Loaded %s (%dx%d)", filename, g->width, g->height);
    }

    return g;
}

// ----------------------------------------------------
// Distance helper
// ----------------------------------------------------
static int get_distance(map_session_data* sd, block_list* bl) {
    if (!sd || !bl) return 999;
    int px = fpm_get_sd_x(sd);
    int py = fpm_get_sd_y(sd);
    int tx = fpm_get_bl_x(bl);
    int ty = fpm_get_bl_y(bl);
    return abs(px - tx) + abs(py - ty);
}

// ----------------------------------------------------
// Move to target using Peregrine
// ----------------------------------------------------
static bool move_to_target(map_session_data* sd, block_list* target) {
    if (!sd || !target || !g_gat_map) return false;

    int sx = fpm_get_sd_x(sd);
    int sy = fpm_get_sd_y(sd);
    int tx = fpm_get_bl_x(target);
    int ty = fpm_get_bl_y(target);

    PStepList steps;
    bool ok = ctx->peregrine->astar(g_gat_map, sx, sy, tx, ty, &steps);
    if (!ok || steps.count <= 0) {
        ctx->log->error("[autoattack] No path to target");
        return false;
    }

    pgn_route_start(ctx, sd, &steps, g_gat_map);
    ctx->log->info("[autoattack] Moving to target at (%d,%d)", tx, ty);
    return true;
}

// ----------------------------------------------------
// Attack tick
// ----------------------------------------------------
static int aa_tick(int tid, uint64_t tick, int id, intptr_t data) {
    if (!g_enabled || g_account_id < 0) return 0;

    map_session_data* sd = ctx->player->map_id2sd(g_account_id);
    if (!sd) {
        g_enabled = false;
        ctx->log->info("[autoattack] Player logged out");
        return 0;
    }

    switch (g_state) {
        case STATE_SEARCHING: {
            block_list* mob = ctx->combat->get_nearest_mob(sd, 15);
            if (mob) {
                g_current_target = mob;
                ctx->log->info("[autoattack] Target found: %d", fpm_get_bl_id(mob));
                int dist = get_distance(sd, mob);
                if (dist <= 2) {
                    g_state = STATE_ATTACKING;
                } else {
                    if (move_to_target(sd, mob))
                        g_state = STATE_ATTACKING;
                }
            }
            break;
        }

        case STATE_ATTACKING: {
            if (!g_current_target) {
                g_state = STATE_SEARCHING;
                break;
            }
            int dist = get_distance(sd, g_current_target);
            if (dist > 2) {
                move_to_target(sd, g_current_target);
            } else if (tick - g_last_attack >= 500) {
                int result = ctx->combat->unit_attack(sd, g_current_target);
                g_last_attack = tick;
                if (result != 0) {
                    ctx->log->info("[autoattack] Target eliminated");
                    g_current_target = nullptr;
                    g_state = STATE_SEARCHING;
                }
            }
            break;
        }
    }

    ctx->timer->add_timer(tick + 200, aa_tick, 0, 0);
    return 0;
}

// ----------------------------------------------------
// @aa command
// ----------------------------------------------------
static int cmd_autoattack(map_session_data* sd, const char* cmd, const char* msg) {
    if (!sd) return -1;

    if (msg && strcmp(msg, "off") == 0) {
        g_enabled = false;
        pgn_route_stop();
        ctx->player->send_message(sd, "[AutoAttack] DISABLED");
        return 0;
    }

    if (!g_enabled) {
        g_account_id = ctx->player->get_account_id(sd);
        g_gat_map = load_gat_for_sd(sd);
        if (!g_gat_map) {
            ctx->player->send_message(sd, "[AutoAttack] Failed to load map data");
            return -1;
        }
        g_enabled = true;
        g_state = STATE_SEARCHING;
        ctx->timer->add_timer(ctx->timer->gettick() + 100, aa_tick, 0, 0);
        ctx->player->send_message(sd, "[AutoAttack] ENABLED");
        ctx->log->info("[autoattack] Enabled");
    } else {
        g_enabled = false;
        pgn_route_stop();
        if (g_gat_map) {
            ctx->peregrine->free_gat(g_gat_map);
            g_gat_map = nullptr;
        }
        ctx->player->send_message(sd, "[AutoAttack] DISABLED");
        ctx->log->info("[autoattack] Disabled");
    }
    return 0;
}

// ----------------------------------------------------
// Plugin init/final
// ----------------------------------------------------
extern "C" {
bool plugin_init(const PluginContext* unused) {
    (void)unused;  // ignore loader context
    ctx = falconpm_get_context();   // ✅ always use global context
    ctx->atcommand->add("aa", cmd_autoattack);
    ctx->log->info("[autoattack] Plugin initialized (@aa)");
    return true;
}

void plugin_final() {
    if (g_gat_map && ctx && ctx->peregrine) {
        ctx->peregrine->free_gat(g_gat_map);
        g_gat_map = nullptr;
    }
    g_enabled = false;
}

PluginDescriptor PLUGIN = {
    "autoattack",
    "0.2-simplified",
    nullptr,
    plugin_init,
    plugin_final
};

}
