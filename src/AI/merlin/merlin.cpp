#include "mln_api.h"
#include "mln_target.h"
#include "mln_attack.h"
#include "../../infra/plugin_api.h"
#include "../../plugins/autoattack/autoattack_globals.h"
#include "../peregrine/pgn_path.h"
#include "../peregrine/pgn_gat.h"
#include "../../core/falconpm.hpp"
#include <cstdio>
#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
extern const PluginContext* falconpm_get_context(void);

// Merlin's global state with target tracking
static block_list* current_target = nullptr;
static bool moving_to_target = false;
static void* last_killed_target = nullptr;
static uint64_t last_kill_time = 0;
static uint64_t last_target_search = 0;
static uint64_t last_roam_move = 0;

static bool is_target_valid(block_list* target) {
    if (!target) return false;
    
    const PluginContext* ctx = falconpm_get_context();
    if (!ctx) return false;
    
    uint64_t now = ctx->timer->gettick();
    
    // Skip recently killed targets for 5 seconds
    if (target == last_killed_target && now - last_kill_time < 5000) {
        return false;
    }
    
    return true;
}

static bool find_roaming_destination(map_session_data* sd, const PluginContext* c, int* rx, int* ry) {
    int sx = fpm_get_sd_x(sd);
    int sy = fpm_get_sd_y(sd);
    
    // Try to find a walkable spot within reasonable distance
    for (int attempts = 0; attempts < 10; attempts++) {
        *rx = sx + ((rand() % 30) - 15); // ±15 cells
        *ry = sy + ((rand() % 30) - 15); // ±15 cells
        
        if (c->peregrine->is_walkable(g_autoattack_map, *rx, *ry)) {
            return true;
        }
    }
    return false;
}

void merlin_tick() {
    static MerlinState last_state = MLN_STATE_IDLE;
    MerlinState state = mln_api_get_state();
    const PluginContext* c = falconpm_get_context();

    if (!c || !c->player) return;
    
    // Only operate if AutoAttack enabled us
    if (g_autoattack_account_id < 0 || !g_autoattack_map) return;
    
    map_session_data* sd = c->player->map_id2sd(g_autoattack_account_id);
    if (!sd) return;

    uint64_t now = c->timer->gettick();

    switch (state) {
        case MLN_STATE_ROAMING: {
            if (state != last_state) {
                c->log->info("[Merlin] Combat roaming active");
            }
            
            // Prevent rapid target searching
            if (now - last_target_search < 2000) {
                break;
            }
            
            // Look for valid targets
            block_list* mob = c->combat->get_nearest_mob(sd, 15);
            if (mob && is_target_valid(mob)) {
                last_target_search = now;
                current_target = mob;
                c->log->info("[Merlin] Target acquired - initiating attack sequence");
                
                // Calculate distance and move if necessary
                int sx = fpm_get_sd_x(sd), sy = fpm_get_sd_y(sd);
                int tx = fpm_get_bl_x(mob), ty = fpm_get_bl_y(mob);
                int dist = abs(sx - tx) + abs(sy - ty);
                
                if (dist > 2) {
                    // Need to move closer
                    PStepList steps;
                    if (c->peregrine->astar(g_autoattack_map, sx, sy, tx, ty, &steps)) {
                        c->peregrine->route_start(c, sd, &steps, g_autoattack_map);
                        moving_to_target = true;
                        c->log->info("[Merlin] Moving to target (distance: %d)", dist);
                    } else {
                        c->log->info("[Merlin] Cannot reach target - ignoring");
                        current_target = nullptr;
                        break;
                    }
                } else {
                    // Close enough to attack
                    moving_to_target = false;
                    c->log->info("[Merlin] Target in range - ready to attack");
                }
                
                mln_api_set_state(MLN_STATE_ATTACKING);
                
            } else {
                // No valid targets found - start roaming
                if (now - last_roam_move > 6000) { // Move every 6 seconds
                    int rx, ry;
                    if (find_roaming_destination(sd, c, &rx, &ry)) {
                        PStepList steps;
                        if (c->peregrine->astar(g_autoattack_map, 
                                               fpm_get_sd_x(sd), fmp_get_sd_y(sd), 
                                               rx, ry, &steps)) {
                            c->peregrine->route_start(c, sd, &steps, g_autoattack_map);
                            c->log->info("[Merlin] No targets - roaming to (%d,%d)", rx, ry);
                            last_roam_move = now;
                        }
                    } else {
                        c->log->info("[Merlin] No walkable roaming destination found");
                        last_roam_move = now; // Prevent spam
                    }
                }
            }
            break;
        }

        case MLN_STATE_ATTACKING: {
            if (state != last_state) {
                c->log->info("[Merlin] Engaging target");
            }
            
            if (!current_target) {
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
            
            // Execute attack sequence
            if (!mln_attack_in_progress()) {
                // Start new attack
                if (!mln_attack_start(current_target)) {
                    c->log->info("[Merlin] Attack failed to start - target lost");
                    current_target = nullptr;
                    mln_api_set_state(MLN_STATE_ROAMING);
                }
            } else if (mln_attack_done()) {
                // Attack completed
                c->log->info("[Merlin] Target eliminated - searching for next target");
                
                // Track the killed target globally
                last_killed_target = current_target;
                last_kill_time = now;
                
                // Clear current target and return to roaming
                current_target = nullptr;
                mln_api_set_state(MLN_STATE_ROAMING);
            }
            break;
        }

        case MLN_STATE_IDLE: {
            if (state != last_state) {
                c->log->info("[Merlin] Combat disabled");
            }
            
            // Clean up any active states
            if (current_target) {
                current_target = nullptr;
                moving_to_target = false;
                c->peregrine->route_stop();
            }
            
            // Reset tracking
            last_killed_target = nullptr;
            last_kill_time = 0;
            last_target_search = 0;
            last_roam_move = 0;
            break;
        }
    }

    last_state = state;
}

#ifdef __cplusplus
}
#endif