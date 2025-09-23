#include "mln_api.h"
#include "mln_target.h"
#include "mln_attack.h"
#include "../../infra/plugin_api.h"
#include "../../plugins/autoattack/autoattack_globals.h"
#include "../peregrine/pgn_path.h"
#include "../peregrine/pgn_gat.h"
#include "../../core/falconpm.hpp"
#include <cstdio>
#include <cstdlib>  // ADD this line for abs() function

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
extern const PluginContext* falconpm_get_context(void);

// Merlin's private state
static block_list* current_target = nullptr;
static bool moving_to_target = false;

void merlin_tick() {
    static MerlinState last_state = MLN_STATE_IDLE;
    MerlinState state = mln_api_get_state();
    const PluginContext* c = falconpm_get_context();

    if (!c || !c->player) return;
    
    // Only operate if AutoAttack enabled us
    if (g_autoattack_account_id < 0 || !g_autoattack_map) return;
    
    map_session_data* sd = c->player->map_id2sd(g_autoattack_account_id);
    if (!sd) return;

    switch (state) {
        case MLN_STATE_ROAMING: {
            if (state != last_state) {
                c->log->info("[Merlin] Combat roaming active");
            }
            
            // Look for targets (like OpenKore's target detection)
            block_list* mob = c->combat->get_nearest_mob(sd, 15);
            if (mob) {
                current_target = mob;
                c->log->info("[Merlin] Target acquired - initiating attack sequence");
                
                // Check if we need to move to target
                int sx = fpm_get_sd_x(sd), sy = fpm_get_sd_y(sd);
                int tx = fpm_get_bl_x(mob), ty = fpm_get_bl_y(mob);
                
                // Calculate distance (simple Manhattan)
                int dist = abs(sx - tx) + abs(sy - ty);
                
                if (dist > 2) {
                    // Need to move closer (like OpenKore's ai_route)
                    PStepList steps;
                    if (c->peregrine->astar(g_autoattack_map, sx, sy, tx, ty, &steps)) {
                        c->peregrine->route_start(c, sd, &steps, g_autoattack_map);
                        moving_to_target = true;
                        c->log->info("[Merlin] Moving to target");
                    } else {
                        c->log->info("[Merlin] Cannot reach target - ignoring");
                        current_target = nullptr;
                    }
                } else {
                    // Close enough to attack
                    moving_to_target = false;
                }
                
                mln_api_set_state(MLN_STATE_ATTACKING);
            }
            break;
        }

        case MLN_STATE_ATTACKING: {
            if (state != last_state) {
                c->log->info("[Merlin] Engaging target");
            }
            
            if (!current_target) {
                // Target lost
                c->log->info("[Merlin] Target lost - resuming roaming");
                mln_api_set_state(MLN_STATE_ROAMING);
                break;
            }
            
            // Check if still moving to target
            if (moving_to_target && c->peregrine->route_active()) {
                // Still moving, wait
                break;
            }
            moving_to_target = false;
            
            // Execute attack (like OpenKore's attack execution)
            if (!mln_attack_in_progress()) {
                if (!mln_attack_start(current_target)) {
                    c->log->info("[Merlin] Attack failed - target lost");
                    current_target = nullptr;
                    mln_api_set_state(MLN_STATE_ROAMING);
                }
            } else if (mln_attack_done()) {
                c->log->info("[Merlin] Target eliminated");
                current_target = nullptr;
                mln_api_set_state(MLN_STATE_ROAMING);
            }
            break;
        }

        case MLN_STATE_IDLE: {
            // Combat disabled - do nothing
            if (current_target) {
                current_target = nullptr;
                moving_to_target = false;
                c->peregrine->route_stop();
            }
            break;
        }
    }

    last_state = state;
}

#ifdef __cplusplus
}
#endif