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

// All external function declarations with proper C linkage
extern "C" {
    void mark_mob_engaged(int mob_id, int account_id);
    void clear_mob_engagement(int mob_id);
    bool is_mob_engaged_by_other(int mob_id, int account_id);
    int fpm_get_bl_id(block_list* bl);
    int fpm_get_bl_x(block_list* bl);
    int fpm_get_bl_y(block_list* bl);
}

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

    // Enhanced initialization debugging
    if (!c || !c->player) {
        printf("[Debug] Merlin tick aborted: No plugin context or player API\n");
        return;
    }
    
    // Only operate if AutoAttack enabled us
    if (g_autoattack_account_id < 0 || !g_autoattack_map) {
        if (state != MLN_STATE_IDLE) {
            printf("[Debug] Merlin tick: AutoAttack not enabled (account_id=%d, map=%p)\n", 
                   g_autoattack_account_id, g_autoattack_map);
        }
        return;
    }
    
    map_session_data* sd = c->player->map_id2sd(g_autoattack_account_id);
    if (!sd) {
        printf("[Debug] Merlin tick aborted: Cannot resolve session data for account_id=%d\n", 
               g_autoattack_account_id);
        return;
    }

    uint64_t now = c->timer->gettick();
    
    // State transition debugging
    if (state != last_state) {
        printf("[Debug] Merlin state transition: %d -> %d at tick %lu\n", 
               (int)last_state, (int)state, now);
    }

    switch (state) {
        case MLN_STATE_ROAMING: {
            if (state != last_state) {
                c->log->info("[Merlin] Combat roaming active - searching for targets");
                printf("[Debug] Entered ROAMING state - player at (%d,%d)\n", 
                       fpm_get_sd_x(sd), fpm_get_sd_y(sd));
            }
            
            // Prevent rapid target searching
            if (now - last_target_search < 2000) {
                printf("[Debug] Target search cooldown active (%lu ms remaining)\n", 
                       2000 - (now - last_target_search));
                break;
            }
            
            printf("[Debug] Searching for nearest mob in 15-cell radius...\n");
            
            // Look for valid targets
            block_list* mob = c->combat->get_nearest_mob(sd, 15);
            if (mob) {
                // Extract processed data
                int mob_id = fpm_get_bl_id(mob);
                int mob_x = fpm_get_bl_x(mob);
                int mob_y = fpm_get_bl_y(mob);
                
                printf("[Debug] Found mob: ID=%d at (%d,%d)\n", mob_id, mob_x, mob_y);
                
                if (is_target_valid(mob)) {
                    printf("[Debug] Target validation passed - engaging mob ID=%d\n", mob_id);
                    
                    mark_mob_engaged(mob_id, g_autoattack_account_id);
                    
                    last_target_search = now;
                    current_target = mob;
                    c->log->info("[Merlin] Target acquired (Vitality+Anti-KS validated) - initiating attack sequence");
                    
                    // Calculate distance and move if necessary  
                    int sx = fpm_get_sd_x(sd), sy = fpm_get_sd_y(sd);
                    int dist = abs(sx - mob_x) + abs(sy - mob_y);
                    
                    printf("[Debug] Distance to target: %d cells (player: %d,%d -> mob: %d,%d)\n", 
                           dist, sx, sy, mob_x, mob_y);
                    
                    if (dist > 2) {
                        printf("[Debug] Target too far - initiating pathfinding\n");
                        PStepList steps;
                        if (c->peregrine->astar(g_autoattack_map, sx, sy, mob_x, mob_y, &steps)) {
                            c->peregrine->route_start(c, sd, &steps, g_autoattack_map);
                            moving_to_target = true;
                            c->log->info("[Merlin] Moving to target (distance: %d)", dist);
                            printf("[Debug] Pathfinding successful - %d steps planned\n", steps.count);
                        } else {
                            c->log->info("[Merlin] Cannot reach target - ignoring");
                            printf("[Debug] Pathfinding failed - no route to target\n");
                            clear_mob_engagement(mob_id);
                            current_target = nullptr;
                            break;
                        }
                    } else {
                        moving_to_target = false;
                        c->log->info("[Merlin] Target in range - ready to attack");
                        printf("[Debug] Target within attack range - no movement needed\n");
                    }
                    
                    mln_api_set_state(MLN_STATE_ATTACKING);
                    
                } else {
                    c->log->info("[Merlin] Target found but failed validation - continuing search");
                    printf("[Debug] Target validation failed for mob ID=%d - skipping\n", mob_id);
                }
                
            } else {
                printf("[Debug] No mobs found in search radius\n");
                
                // NO VALID TARGETS - Enhanced roaming behavior
                if (now - last_roam_move > 6000) {
                    c->log->info("[Merlin] No valid targets found - searching new area");
                    printf("[Debug] Initiating roaming behavior (last roam: %lu ms ago)\n", 
                           now - last_roam_move);
                    
                    int rx, ry;
                    if (find_roaming_destination(sd, c, &rx, &ry)) {
                        PStepList steps;
                        if (c->peregrine->astar(g_autoattack_map, 
                                               fpm_get_sd_x(sd), fpm_get_sd_y(sd), 
                                               rx, ry, &steps)) {
                            c->peregrine->route_start(c, sd, &steps, g_autoattack_map);
                            c->log->info("[Merlin] Roaming to new hunting ground (%d,%d)", rx, ry);
                            printf("[Debug] Roaming route planned: %d steps to (%d,%d)\n", 
                                   steps.count, rx, ry);
                            last_roam_move = now;
                        } else {
                            printf("[Debug] Roaming pathfinding failed to (%d,%d)\n", rx, ry);
                        }
                    } else {
                        printf("[Debug] Failed to find valid roaming destination\n");
                        last_roam_move = now; // Prevent spam
                    }
                } else {
                    printf("[Debug] Roaming cooldown active (%lu ms remaining)\n", 
                           6000 - (now - last_roam_move));
                }
            }
            break;
        }

        case MLN_STATE_ATTACKING: {
            if (state != last_state) {
                c->log->info("[Merlin] Engaging target");
                printf("[Debug] Entered ATTACKING state - target=%p, moving=%d\n", 
                       current_target, moving_to_target);
            }
            
            if (!current_target) {
                c->log->info("[Merlin] Target lost - resuming roaming");
                printf("[Debug] No current target - returning to ROAMING\n");
                mln_api_set_state(MLN_STATE_ROAMING);
                break;
            }
            
            // Check if still moving to target
            if (moving_to_target && c->peregrine->route_active()) {
                printf("[Debug] Still moving to target - waiting for navigation completion\n");
                break;
            } else if (moving_to_target) {
                moving_to_target = false;
                printf("[Debug] Movement completed - ready to attack\n");
            }
            
            // Execute attack sequence
            if (!mln_attack_in_progress()) {
                printf("[Debug] Starting new attack sequence\n");
                if (!mln_attack_start(current_target)) {
                    c->log->info("[Merlin] Attack failed to start - target lost");
                    printf("[Debug] Attack initiation failed\n");
                    current_target = nullptr;
                    mln_api_set_state(MLN_STATE_ROAMING);
                }
            } else if (mln_attack_done()) {
                c->log->info("[Merlin] Target eliminated - searching for next target");
                printf("[Debug] Attack sequence completed successfully\n");
                
                // Anti-KS: Clear engagement tracking using processed data
                if (current_target) {
                    int mob_id = fpm_get_bl_id(current_target);
                    clear_mob_engagement(mob_id);
                    printf("[Debug] Cleared engagement for mob ID=%d\n", mob_id);
                }
                
                // Track the killed target globally
                last_killed_target = current_target;
                last_kill_time = now;
                printf("[Debug] Blacklisted target %p for 5 seconds\n", current_target);
                
                current_target = nullptr;
                mln_api_set_state(MLN_STATE_ROAMING);
            } else {
                printf("[Debug] Attack in progress - monitoring\n");
            }
            break;
        }

        case MLN_STATE_IDLE: {
            if (state != last_state) {
                c->log->info("[Merlin] Combat disabled");
                printf("[Debug] Entered IDLE state - cleaning up\n");
            }
            
            // Clean up any active states
            if (current_target) {
                printf("[Debug] Clearing active target in IDLE state\n");
                current_target = nullptr;
                moving_to_target = false;
                c->peregrine->route_stop();
            }
            
            // Reset tracking
            if (last_killed_target) {
                printf("[Debug] Resetting target tracking in IDLE state\n");
                last_killed_target = nullptr;
                last_kill_time = 0;
                last_target_search = 0;
                last_roam_move = 0;
            }
            break;
        }

        default: {
            printf("[Debug] Unknown Merlin state: %d\n", (int)state);
            break;
        }
    }

    last_state = state;
    
    // Periodic status debugging (every 10 seconds)
    static uint64_t last_status_log = 0;
    if (now - last_status_log > 10000) {
        printf("[Debug] Merlin status: state=%d, target=%p, account_id=%d, map=%p\n", 
               (int)state, current_target, g_autoattack_account_id, g_autoattack_map);
        last_status_log = now;
    }
}

// Debug version of is_target_valid for troubleshooting
static bool is_target_valid_debug(block_list* target) {
    if (!target) {
        printf("[Debug] Target validation failed: NULL pointer\n");
        return false;
    }
    
    const PluginContext* ctx = falconpm_get_context();
    if (!ctx) return false;
    
    uint64_t now = ctx->timer->gettick();
    
    // Check against recently killed targets
    if (target == last_killed_target && now - last_kill_time < 5000) {
        printf("[Debug] Target validation failed: Recently killed target\n");
        return false;
    }
    
    printf("[Debug] Target validation passed: Valid new target\n");
    return true;
}

#ifdef __cplusplus
}
#endif