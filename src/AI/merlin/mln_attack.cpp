#include "mln_attack.h"
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include <cstdio>

// Reference core globals  
extern int g_autoattack_account_id;      
extern GatMap* g_autoattack_map;        

// Attack state  
static bool attack_active = false;
static void* current_target = nullptr;  
static int attack_counter = 0;
static uint64_t last_attack_time = 0;
static uint64_t attack_start_time = 0;  
static int attack_attempts = 0;         

extern const PluginContext* falconpm_get_context(void);

bool mln_attack_start(void* mob) {
    if (!mob) return false;
    
    current_target = mob;
    attack_active = true;
    attack_counter++;
    attack_start_time = 0;  // Reset timing
    attack_attempts = 0;    // Reset attempts
    
    printf("[Merlin] Attack sequence #%d initiated\n", attack_counter);
    return true;
}

bool mln_attack_in_progress(void) {
    if (!attack_active || !current_target) return false;
    
    const PluginContext* ctx = falconpm_get_context();
    if (!ctx || g_autoattack_account_id < 0) return false;
    
    struct map_session_data* sd = ctx->player->map_id2sd(g_autoattack_account_id);
    if (!sd) return false;
    
    uint64_t now = ctx->timer->gettick();
    
    // Initialize attack start time
    if (attack_start_time == 0) {
        attack_start_time = now;
        attack_attempts = 0;
    }
    
    // Attack every 500ms
    if (now - last_attack_time >= 500) {
        int result = ctx->combat->unit_attack(sd, (struct block_list*)current_target);
        last_attack_time = now;
        attack_attempts++;
        
        if (result != 0) {
            // Target might be dead - but wait for minimum attack duration
            if (now - attack_start_time > 1500 || attack_attempts >= 5) {
                printf("[Merlin] Attack completed - target eliminated\n");
                return false;
            }
        }
        
        printf("[Merlin] Attack executed (attempt %d)\n", attack_attempts);
    }
    
    // Maximum attack duration safety net
    if (now - attack_start_time > 10000) { // 10 seconds max
        printf("[Merlin] Attack timeout - switching targets\n");
        return false;
    }
    
    return true;
}

bool mln_attack_done(void) {
    if (attack_active && !mln_attack_in_progress()) {
        attack_active = false;
        current_target = nullptr;
        attack_start_time = 0;  // Reset timing
        attack_attempts = 0;    // Reset attempts
        return true;
    }
    return false;
}