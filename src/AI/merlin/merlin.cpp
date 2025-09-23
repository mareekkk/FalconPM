// /home/marek/FalconPM/src/AI/merlin/merlin.cpp
#include "mln_api.h"
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include "mln_attack.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include "../peregrine/pgn_api.h"

// ----------------------------------------------------
// Globals & State
// ----------------------------------------------------
static MerlinState g_merlin_state = MLN_STATE_IDLE;
static const PluginContext* ctx = nullptr;

// Persist target by ID, not pointer (avoid stale pointer after movement)
static int   current_target_id  = -1;
static void* current_target_ptr = nullptr; // transient; resolved fresh pre-attack

static int      last_killed_target_id = -1;
static uint64_t last_kill_time        = 0;

static bool moving_to_target = false;
static int  kill_location_x  = -1;
static int  kill_location_y  = -1;
static bool must_move_after_kill = false;

static uint64_t last_target_search = 0;
static uint64_t last_roam_move     = 0;

// 🔹 extern declarations provided by your core
extern int    g_autoattack_account_id;
extern GatMap* g_autoattack_map;

// Bootstrap helpers
extern "C" {
    int  fpm_get_bl_x(block_list* bl);
    int  fpm_get_bl_y(block_list* bl);
    int  fpm_get_bl_id(block_list* bl);
    int  fpm_get_sd_x(map_session_data* sd);
    int  fpm_get_sd_y(map_session_data* sd);
    int  fpm_get_sd_m(map_session_data* sd);
    const char* fpm_get_map_name(int m);

    bool fpm_is_mob_alive(block_list* mob);
    int  fpm_get_mob_hp_percent(block_list* mob);

    void clear_mob_engagement(int mob_id);
}

// ----------------------------------------------------
// Helpers
// ----------------------------------------------------
static bool is_target_valid_by_ptr(block_list* target) {
    if (!target) return false;
    const int mob_id = fpm_get_bl_id(target);
    if (!mob_id) return false;

    // avoid re-targeting the same corpse right after a kill
    if (last_killed_target_id >= 0 && mob_id == last_killed_target_id) {
        const uint64_t now = ctx->timer->gettick();
        if (now - last_kill_time < 5000) return false;
    }
    return true;
}

// Resolve a fresh pointer from stored id (safer than keeping raw pointer)
static void* resolve_target_ptr(map_session_data* sd, int target_id) {
    if (!sd || target_id < 0 || !ctx || !ctx->combat) return nullptr;

    block_list* m = ctx->combat->get_nearest_mob(sd, 50);
    if (m && fpm_get_bl_id(m) == target_id) return (void*)m;

    for (int r = 60; r <= 120; r += 30) {
        block_list* mm = ctx->combat->get_nearest_mob(sd, r);
        if (mm && fpm_get_bl_id(mm) == target_id) return (void*)mm;
    }
    return nullptr;
}

// ----------------------------------------------------
// Main Tick
// ----------------------------------------------------
void merlin_tick() {
    ctx = falconpm_get_context();
    if (!ctx || g_autoattack_account_id < 0) return;

    map_session_data* sd = ctx->player->map_id2sd(g_autoattack_account_id);
    if (!sd) return;

    MerlinState state = mln_api_get_state();
    uint64_t now = ctx->timer->gettick();

    switch (state) {
    case MLN_STATE_ROAMING: {
        // throttle target search
        if (now - last_target_search < 3000) return;
        last_target_search = now;

        block_list* mob = ctx->combat->get_nearest_mob(sd, 15);
        if (mob && is_target_valid_by_ptr(mob)) {
            // store id only; resolve pointer when we’re ready to attack
            current_target_id  = fpm_get_bl_id(mob);
            current_target_ptr = nullptr;

            const int sx = fpm_get_sd_x(sd);
            const int sy = fpm_get_sd_y(sd);
            const int tx = fpm_get_bl_x(mob);
            const int ty = fpm_get_bl_y(mob);

            PStepList steps{};
            if (ctx->peregrine && ctx->peregrine->astar && ctx->peregrine->route_start) {
                if (ctx->peregrine->astar(g_autoattack_map, sx, sy, tx, ty, &steps)) {
                    moving_to_target = true;
                    ctx->peregrine->route_start(ctx, sd, &steps, g_autoattack_map);
                    ctx->log->info("[Merlin] Moving toward target at (%d,%d)", tx, ty);
                    mln_api_set_state(MLN_STATE_ATTACKING);
                }
                if (ctx->peregrine->free_steps) {
                    ctx->peregrine->free_steps(&steps);
                }
            }
        } else {
            if (now - last_roam_move > 8000) {
                ctx->log->info("[Merlin] Roaming around (stubbed)");
                last_roam_move = now;
            }
        }
        break;
    }

    case MLN_STATE_ATTACKING: {
        if (!sd) break;

        // Wait until Peregrine has finished routing
        if (moving_to_target) {
            bool route_is_active = false;
            if (ctx->peregrine && ctx->peregrine->route_active) {
                route_is_active = ctx->peregrine->route_active(); // ← no args
            }
            if (route_is_active) {
                printf("[Debug] Still moving to target - waiting for navigation completion\n");
                return;
            } else {
                moving_to_target = false;
                printf("[Debug] Movement completed - ready to attack\n");
            }
        }

        // resolve fresh pointer prior to attacking
        if (current_target_id >= 0 && !current_target_ptr) {
            current_target_ptr = resolve_target_ptr(sd, current_target_id);
            if (!current_target_ptr) {
                ctx->log->info("[Merlin] Target pointer could not be resolved (died or moved away).");
                current_target_id = -1;
                mln_api_set_state(MLN_STATE_ROAMING);
                break;
            }
        }

        if (!mln_attack_in_progress() && !mln_attack_done()) {
            printf("[Debug] Starting new attack sequence\n");
            if (!mln_attack_start(current_target_ptr)) {
                ctx->log->info("[Merlin] Attack failed to start - target lost");
                current_target_ptr = nullptr;
                current_target_id  = -1;
                mln_api_set_state(MLN_STATE_ROAMING);
            }
        } else if (mln_attack_done()) {
            ctx->log->info("[Merlin] Target eliminated");

            if (current_target_ptr) {
                const int mob_id = fpm_get_bl_id((block_list*)current_target_ptr);
                kill_location_x = fpm_get_bl_x((block_list*)current_target_ptr);
                kill_location_y = fpm_get_bl_y((block_list*)current_target_ptr);
                must_move_after_kill = false;
                last_killed_target_id = mob_id;
            }

            last_kill_time = now;
            current_target_ptr = nullptr;
            current_target_id  = -1;
            mln_api_set_state(MLN_STATE_ROAMING);
        } else {
            printf("[Debug] Attack in progress - monitoring\n");
            (void)mln_attack_in_progress();
        }
        break;
    }

    case MLN_STATE_IDLE:
        // nothing
        break;

    default:
        printf("[Debug] Unknown Merlin state: %d\n", (int)state);
        break;
    }
}
