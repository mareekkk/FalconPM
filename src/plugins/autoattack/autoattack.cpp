// src/plugins/autoattack/autoattack.cpp
// FalconPM AutoAttack Plugin
// Uses Peregrine for movement and basic attack through APIs

#include "../../infra/plugin_api.h"
#include "../../AI/peregrine/peregrine.h"
#include "../../AI/peregrine/pgn_path.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>   // for snprintf, fprintf, stderr


// Shim functions from falconpm_bootstrap.cpp
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
extern "C" const PluginContext* falconpm_get_context(void);
static const PluginContext* g_ctx = nullptr;
static int g_account_id = -1;
static bool g_enabled = false;
static block_list* g_current_target = nullptr;
static GatMap* g_gat_map = nullptr;
static uint64_t g_last_attack = 0;
static int g_attack_count = 0;
static int g_kill_count = 0;

// State machine
enum AttackState {
    STATE_IDLE,
    STATE_SEARCHING,
    STATE_MOVING,
    STATE_ATTACKING
};
static AttackState g_state = STATE_IDLE;

// ----------------------------------------------------
// GAT map loading (cached per map)
// ----------------------------------------------------
static GatMap* load_gat_for_sd(map_session_data* sd) {
    if (!sd || !g_ctx || !g_ctx->peregrine) return nullptr;

    int map_index = fpm_get_sd_m(sd);
    const char* mapname = fpm_get_map_name(map_index);

    char filename[512];
    snprintf(filename, sizeof(filename), "./data/gat/%s.gat", mapname);

    GatMap* gat = g_ctx->peregrine->load_gat(filename);
    if (!gat) {
        g_ctx->log->error("[autoattack] Failed to load GAT: %s", filename);
    }
    return gat;
}

// ----------------------------------------------------
// Distance calculation
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

    // Adjust target to be 1–2 cells away (melee range)
    if (tx > sx) tx -= 1;
    else if (tx < sx) tx += 1;
    if (ty > sy) ty -= 1;
    else if (ty < sy) ty += 1;

    // Bounds check using shim
    int map_index = fpm_get_sd_m(sd);
    if (tx < 0) tx = 0;
    if (ty < 0) ty = 0;
    if (tx >= fpm_get_map_width(map_index)) tx = fpm_get_map_width(map_index) - 1;
    if (ty >= fpm_get_map_height(map_index)) ty = fpm_get_map_height(map_index) - 1;

    // Use Peregrine A* pathfinding
    PStepList steps;
    bool ok = g_ctx->peregrine->astar(g_gat_map, sx, sy, tx, ty, &steps);

    if (!ok || steps.count <= 0) {
        g_ctx->log->error("[autoattack] No path to target");
        return false;
    }

    // Start Peregrine route
    pgn_route_start(g_ctx, sd, &steps, g_gat_map);

    g_ctx->log->info("[autoattack] Moving to target at (%d,%d)", tx, ty);
    return true;
}

// ----------------------------------------------------
// Attack tick handler
// ----------------------------------------------------
static int aa_tick(int tid, uint64_t tick, int id, intptr_t data) {
    if (!g_enabled || g_account_id < 0) return 0;

    map_session_data* sd = g_ctx->player->map_id2sd(g_account_id);
    if (!sd) {
        g_ctx->log->info("[autoattack] Player logged out");
        g_enabled = false;
        g_account_id = -1;
        return 0;
    }

    switch (g_state) {
        case STATE_IDLE:
            g_state = STATE_SEARCHING;
            break;

        case STATE_SEARCHING: {
            block_list* mob = g_ctx->combat->get_nearest_mob(sd, 15);
            if (mob) {
                g_current_target = mob;
                int target_id = fpm_get_bl_id(mob);
                g_ctx->log->info("[autoattack] Target found: ID %d", target_id);

                int dist = get_distance(sd, mob);
                if (dist <= 2) {
                    g_state = STATE_ATTACKING;
                } else {
                    if (move_to_target(sd, mob)) {
                        g_state = STATE_MOVING;
                    } else {
                        g_current_target = nullptr;
                    }
                }
            }
            break;
        }

        case STATE_MOVING: {
            if (!g_current_target) {
                g_state = STATE_SEARCHING;
                break;
            }

            int dist = get_distance(sd, g_current_target);
            if (dist <= 2) {
                pgn_route_stop();
                g_state = STATE_ATTACKING;
            } else if (dist > 20) {
                g_ctx->log->info("[autoattack] Target too far, searching new one");
                pgn_route_stop();
                g_current_target = nullptr;
                g_state = STATE_SEARCHING;
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
                g_ctx->log->info("[autoattack] Target moved away");
                g_state = STATE_MOVING;
                if (!move_to_target(sd, g_current_target)) {
                    g_current_target = nullptr;
                    g_state = STATE_SEARCHING;
                }
                break;
            }

            if (tick - g_last_attack >= 500) {
                int result = g_ctx->combat->unit_attack(sd, g_current_target);
                if (result == 0) {
                    g_attack_count++;
                    g_last_attack = tick;
                } else {
                    g_ctx->log->info("[autoattack] Target eliminated");
                    g_kill_count++;
                    g_current_target = nullptr;
                    g_state = STATE_SEARCHING;
                }
            }
            break;
        }
    }

    g_ctx->timer->add_timer(tick + 200, aa_tick, 0, 0);
    return 0;
}

// ----------------------------------------------------
// @aa command handler
// ----------------------------------------------------
static int cmd_autoattack(map_session_data* sd, const char* cmd, const char* msg) {
    if (!sd) return -1;

    if (msg && strcmp(msg, "off") == 0) {
        if (g_enabled) {
            g_enabled = false;
            pgn_route_stop();
            g_ctx->player->send_message(sd, "[AutoAttack] DISABLED");
            char buf[256];
            snprintf(buf, sizeof(buf),
                     "[AutoAttack] Stats: %d kills, %d attacks",
                     g_kill_count, g_attack_count);
            g_ctx->player->send_message(sd, buf);
            g_ctx->log->info("[autoattack] Disabled");
        }
        return 0;
    }

    if (msg && strcmp(msg, "status") == 0) {
        char buf[256];
        if (g_enabled) {
            const char* state_name =
                g_state == STATE_IDLE ? "IDLE" :
                g_state == STATE_SEARCHING ? "SEARCHING" :
                g_state == STATE_MOVING ? "MOVING" :
                g_state == STATE_ATTACKING ? "ATTACKING" : "UNKNOWN";
            snprintf(buf, sizeof(buf),
                     "[AutoAttack] ACTIVE | State: %s | Kills: %d | Attacks: %d",
                     state_name, g_kill_count, g_attack_count);
        } else {
            snprintf(buf, sizeof(buf), "[AutoAttack] INACTIVE");
        }
        g_ctx->player->send_message(sd, buf);
        return 0;
    }

    if (!g_enabled) {
        g_account_id = g_ctx->player->get_account_id(sd);
        g_enabled = true;
        g_state = STATE_IDLE;
        g_current_target = nullptr;
        g_attack_count = 0;
        g_kill_count = 0;

        if (g_gat_map) g_ctx->peregrine->free_gat(g_gat_map);
        g_gat_map = load_gat_for_sd(sd);
        if (!g_gat_map) {
            g_ctx->player->send_message(sd, "[AutoAttack] Failed to load map data");
            g_enabled = false;
            return -1;
        }

        g_ctx->timer->add_timer(g_ctx->timer->gettick() + 100, aa_tick, 0, 0);

        g_ctx->player->send_message(sd, "[AutoAttack] ENABLED");
        g_ctx->log->info("[autoattack] Enabled for account %d", g_account_id);
    } else {
        g_enabled = false;
        pgn_route_stop();
        g_ctx->player->send_message(sd, "[AutoAttack] DISABLED");
        g_ctx->log->info("[autoattack] Disabled");
    }

    return 0;
}

// ----------------------------------------------------
// Plugin initialization
// ----------------------------------------------------
extern "C" {

bool plugin_init(const PluginContext* ctx) {
    g_ctx = falconpm_get_context();
    if (!g_ctx) {
        fprintf(stderr, "[autoattack] Failed to get context\n");
        return false;
    }

    if (!g_ctx->atcommand || !g_ctx->player || !g_ctx->timer ||
        !g_ctx->combat || !g_ctx->peregrine) {
        fprintf(stderr, "[autoattack] Missing required APIs\n");
        return false;
    }

    g_ctx->atcommand->add("aa", cmd_autoattack);
    g_ctx->log->info("[autoattack] Plugin initialized (@aa)");
    return true;
}

void plugin_final() {
    if (g_enabled) pgn_route_stop();
    if (g_gat_map && g_ctx && g_ctx->peregrine) {
        g_ctx->peregrine->free_gat(g_gat_map);
    }
    g_enabled = false;
    g_account_id = -1;
}

PluginDescriptor PLUGIN = {
    "autoattack",
    "1.0",
    nullptr,
    plugin_init,
    plugin_final
};

} // extern "C"
