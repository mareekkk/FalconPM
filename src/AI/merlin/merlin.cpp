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
static int kill_location_x = -1;
static int kill_location_y = -1;
static bool must_move_after_kill = false;

static bool find_roaming_destination_around(map_session_data* sd, const PluginContext* c, int prefer_x, int prefer_y, int* rx, int* ry) {
    // Try to find walkable spot around preferred location
    for (int attempts = 0; attempts < 15; attempts++) {
        int test_x = prefer_x + ((rand() % 10) - 5);
        int test_y = prefer_y + ((rand() % 10) - 5);
        
        if (c->peregrine->is_walkable(g_autoattack_map, test_x, test_y)) {
            *rx = test_x;
            *ry = test_y;
            return true;
        }
    }
    return false;
}

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
    
    // CRITICAL: Only operate when character is fully logged in and on a map
    if (fpm_get_sd_m(sd) < 0) {
        return; // Not on a valid map - likely in char select or loading
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
            c->log->info("[Merlin] Combat roaming active");
        }
        
        // MANDATORY POST-KILL MOVEMENT // replace this
        if (must_move_after_kill) {
            // [DISABLED] Skipping mandatory movement logic to avoid conflict
            must_move_after_kill = false;
            c->log->info("[Merlin] Mandatory movement disabled - resuming target search immediately");
        } // with this
        
        // Continue with normal target search only after mandatory movement
        if (!must_move_after_kill) {
            // Prevent rapid target searching  
            if (now - last_target_search < 3000) { // Increased to 3 seconds
                return; // Throttle target search
            }
            
            // Look for valid targets
            block_list* mob = c->combat->get_nearest_mob(sd, 15);
            if (mob) {
                if (is_target_valid(mob)) {
                    current_target = mob;
                    int sx = fpm_get_sd_x(sd);
                    int sy = fpm_get_sd_y(sd);
                    int tx = fpm_get_bl_x(mob);
                    int ty = fpm_get_bl_y(mob);

                    PStepList steps;
                    if (c->peregrine->astar(g_autoattack_map, sx, sy, tx, ty, &steps)) {
                        moving_to_target = true;
                        c->peregrine->route_start(c, sd, &steps, g_autoattack_map);
                        c->log->info("[Merlin] Moving toward target at (%d,%d)", tx, ty);
                        mln_api_set_state(MLN_STATE_ATTACKING);
                    } else {
                        c->log->info("[Merlin] Could not path to target — skipping");
                        current_target = nullptr;
                    }
                }
            } else {
                // No targets - explore new area
                if (now - last_roam_move > 8000) {
                    // ... existing roaming logic  
                }
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
            
            if (mln_attack_done()) {
                c->log->info("[Merlin] Target eliminated - mandatory movement required");
                
                // Anti-KS: Clear engagement tracking
                if (current_target) {
                    int mob_id = fpm_get_bl_id(current_target);
                    clear_mob_engagement(mob_id);
                    
                    // MANDATORY: Record kill location for forced movement
                    kill_location_x = fpm_get_bl_x(current_target);
                    kill_location_y = fpm_get_bl_y(current_target);
                    must_move_after_kill = true;
                }
                
                // Track the killed target globally
                last_killed_target = current_target;
                last_kill_time = now;
                
                current_target = nullptr;
                    mln_api_set_state(MLN_STATE_ROAMING);
                } else if (!mln_attack_in_progress()) {
                    printf("[Debug] Starting new attack sequence\n");
                    if (!mln_attack_start(current_target)) {
                        c->log->info("[Merlin] Attack failed to start - target lost");
                        printf("[Debug] Attack initiation failed\n");
                        current_target = nullptr;
                        mln_api_set_state(MLN_STATE_ROAMING);
                    }
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