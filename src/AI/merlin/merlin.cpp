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

// Global state
static MerlinState g_merlin_state = MLN_STATE_IDLE;
static const PluginContext* ctx = nullptr;

static void* current_target = nullptr;
static void* last_killed_target = nullptr;
static uint64_t last_kill_time = 0;

static bool moving_to_target = false;
static int kill_location_x = -1;
static int kill_location_y = -1;
static bool must_move_after_kill = false;

static uint64_t last_target_search = 0;
static uint64_t last_roam_move = 0;

// Forward decls
extern int g_autoattack_account_id;
extern GatMap* g_autoattack_map;

extern "C" {
    int fpm_get_bl_x(block_list* bl);
    int fpm_get_bl_y(block_list* bl);
    int fpm_get_bl_id(block_list* bl);
    int fpm_get_sd_x(map_session_data* sd);
    int fpm_get_sd_y(map_session_data* sd);
    int fpm_get_sd_m(map_session_data* sd);
    const char* fpm_get_map_name(int m);

    void clear_mob_engagement(int mob_id);
}

bool is_target_valid(block_list* target) {
    if (!target) return false;

    int mob_id = fpm_get_bl_id(target);
    if (!mob_id) return false;

    // Don’t re-target same corpse immediately
    if (last_killed_target && target == last_killed_target) {
        uint64_t now = ctx->timer->gettick();
        if (now - last_kill_time < 5000) return false;
    }

    return true;
}

void merlin_tick() {
    ctx = falconpm_get_context();
    if (!ctx || g_autoattack_account_id < 0) return;

    struct map_session_data* sd = ctx->player->map_id2sd(g_autoattack_account_id);
    if (!sd) return;

    MerlinState state = mln_api_get_state();
    uint64_t now = ctx->timer->gettick();

    switch (state) {
    case MLN_STATE_ROAMING: {
        // Throttle mob search
        if (now - last_target_search < 3000) return;
        last_target_search = now;

            block_list* mob = ctx->combat->get_nearest_mob(sd, 15);
    if (mob && is_target_valid(mob)) {
        current_target = mob;
        int sx = fpm_get_sd_x(sd);
        int sy = fpm_get_sd_y(sd);
        int tx = fpm_get_bl_x(mob);
        int ty = fpm_get_bl_y(mob);

        PStepList steps;                                     
    if (ctx->peregrine->astar(g_autoattack_map, sx, sy, tx, ty, &steps)) {  // pass PStepList*
        moving_to_target = true;
        ctx->peregrine->route_start(ctx, sd, &steps, g_autoattack_map); 

            ctx->log->info("[Merlin] Moving toward target at (%d,%d)", tx, ty);
            mln_api_set_state(MLN_STATE_ATTACKING);
        }
    }
    else {
            if (now - last_roam_move > 8000) {
                // roam stub
                ctx->log->info("[Merlin] Roaming around (stubbed)");
                last_roam_move = now;
            }
        }
        break;
    }

    case MLN_STATE_ATTACKING: {
        if (!sd) break;

        if (moving_to_target) {
            printf("[Debug] Still moving to target - waiting for navigation completion\n");
            return;
        }

        if (!mln_attack_in_progress() && !mln_attack_done()) {
            printf("[Debug] Starting new attack sequence\n");
            if (!mln_attack_start(current_target)) {
                ctx->log->info("[Merlin] Attack failed to start - target lost");
                printf("[Debug] Attack initiation failed\n");
                current_target = nullptr;
                mln_api_set_state(MLN_STATE_ROAMING);
            }
        } else if (mln_attack_done()) {
            ctx->log->info("[Merlin] Target eliminated - mandatory movement required");

            if (current_target) {
                int mob_id = fpm_get_bl_id((block_list*)current_target);          // cast
                kill_location_x = fpm_get_bl_x((block_list*)current_target);      // cast
                kill_location_y = fpm_get_bl_y((block_list*)current_target);      // cast

                must_move_after_kill = false;
            }


            last_killed_target = current_target;
            last_kill_time = now;
            current_target = nullptr;
            mln_api_set_state(MLN_STATE_ROAMING);
        } else {
            printf("[Debug] Attack in progress - monitoring\n");
            mln_attack_in_progress();
        }

        break;
    }

    case MLN_STATE_IDLE: {
        // Do nothing
        break;
    }

    default: {
        printf("[Debug] Unknown Merlin state: %d\n", (int)state);
        break;
    }
    }
}
