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
// State
// ----------------------------------------------------
static const PluginContext* ctx = nullptr;
static bool moving_to_target = false;

static int   current_target_id  = -1;   // persist mob ID
static void* current_target_ptr = nullptr; // resolved each tick

extern int g_autoattack_account_id;
extern GatMap* g_autoattack_map;

// ----------------------------------------------------
// Helpers
// ----------------------------------------------------
static void* resolve_target_ptr(map_session_data* sd, int mob_id) {
    if (!sd || mob_id < 0 || !ctx || !ctx->combat) return nullptr;

    block_list* mob = ctx->combat->get_nearest_mob(sd, 30);
    if (mob && fpm_get_bl_id(mob) == mob_id) return mob;

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

    switch (state) {
    case MLN_STATE_ROAMING: {
        block_list* mob = ctx->combat->get_nearest_mob(sd, 15);
        if (mob) {
            current_target_id  = fpm_get_bl_id(mob);
            current_target_ptr = nullptr;

            int sx = fpm_get_sd_x(sd);
            int sy = fpm_get_sd_y(sd);
            int tx = fpm_get_bl_x(mob);
            int ty = fpm_get_bl_y(mob);

            PStepList steps{};
            if (ctx->peregrine->astar &&
                ctx->peregrine->astar(g_autoattack_map, sx, sy, tx, ty, &steps)) {
                ctx->peregrine->route_start(ctx, sd, &steps, g_autoattack_map);
                moving_to_target = true;
                ctx->log->info("[Merlin] ROAMING -> ATTACKING: Moving toward mob %d at (%d,%d)", current_target_id, tx, ty);
            }
            if (ctx->peregrine->free_steps) ctx->peregrine->free_steps(&steps);

            mln_api_set_state(MLN_STATE_ATTACKING);
        } else {
            static uint64_t last_roam_log = 0;
            uint64_t now = ctx->timer->gettick();
            if (now - last_roam_log > 5000) {
                ctx->log->info("[Merlin] ROAMING: No mob found");
                last_roam_log = now;
            }
        }
        break;
    }

    case MLN_STATE_ATTACKING: {
        if (!sd) break;

        if (moving_to_target) {
            if (ctx->peregrine->route_active && ctx->peregrine->route_active()) {
                // Still moving
                static uint64_t last_move_log = 0;
                uint64_t now = ctx->timer->gettick();
                if (now - last_move_log > 2000) {
                    printf("[Debug] Still moving to target...\n");
                    last_move_log = now;
                }
                return;
            }
            moving_to_target = false;
            printf("[Debug] Movement completed - ready to attack\n");
        }

        // Resolve pointer fresh
        if (current_target_id >= 0 && !current_target_ptr) {
            current_target_ptr = resolve_target_ptr(sd, current_target_id);
            if (!current_target_ptr) {
                ctx->log->info("[Merlin] ATTACKING: Target lost before attack start");
                current_target_id = -1;
                mln_api_set_state(MLN_STATE_ROAMING);
                break;
            }
        }

        // Attack logic
        if (!mln_attack_in_progress()) {
            // Attempt new attack
            if (!mln_attack_start(current_target_ptr)) {
                ctx->log->info("[Merlin] ATTACKING: Attack could not start (mob gone)");
                current_target_ptr = nullptr;
                current_target_id  = -1;
                mln_api_set_state(MLN_STATE_ROAMING);
            } else {
                printf("[Debug] ATTACKING: Started attack on mob %d\n", current_target_id);
            }
        } else if (mln_attack_done()) {
            ctx->log->info("[Merlin] ATTACKING: Target eliminated (mob %d)", current_target_id);
            current_target_id  = -1;
            current_target_ptr = nullptr;
            mln_api_set_state(MLN_STATE_ROAMING);
        } else {
            (void)mln_attack_in_progress();
        }
        break;
    }

    case MLN_STATE_IDLE:
        // Do nothing
        break;
    }
}
