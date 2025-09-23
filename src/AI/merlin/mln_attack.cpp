#include "mln_attack.h"
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include <cstdio>

// Reference core globals  
extern int g_autoattack_account_id;      
extern GatMap* g_autoattack_map;  

// All external function declarations with proper C linkage  
extern "C" {
    bool is_mob_engaged_by_other(int mob_id, int account_id);
    int fpm_get_bl_id(block_list* bl);

    // 🔹 Added: mob vitality checks from bootstrap
    bool fpm_is_mob_alive(block_list* mob);
    int  fpm_get_mob_hp_percent(block_list* mob);
}

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
    
    // Anti-KS check (kept)
    int mob_id = fpm_get_bl_id((block_list*)current_target);
    if (is_mob_engaged_by_other(mob_id, g_autoattack_account_id)) {
        printf("[Merlin] Another player engaged our target - disengaging\n");
        attack_active = false;                // NEW: mark attack over
        current_target = nullptr;             // NEW: clear local handle
        return false;
    }

    // Alive/HP validation (uses your bootstrap C-linkage)
    if (!fpm_is_mob_alive((block_list*)current_target)) {
        printf("[Merlin] Target no longer alive - marking as dead\n");
        attack_active = false;                // NEW
        current_target = nullptr;             // NEW
        return false;
    }
    int hp_pct = fpm_get_mob_hp_percent((block_list*)current_target);
    if (hp_pct <= 0) {
        printf("[Merlin] Target HP is 0%% - marking as dead\n");
        attack_active = false;                // NEW
        current_target = nullptr;             // NEW
        return false;
    }
    
    uint64_t now = ctx->timer->gettick();
    
    if (attack_start_time == 0) {
        attack_start_time = now;
        attack_attempts = 0;
    }
    
    // Attack every 500ms (kept)
    if (now - last_attack_time >= 500) {
        int result = ctx->combat->unit_attack(sd, (struct block_list*)current_target);
        last_attack_time = now;
        attack_attempts++;
        
        if (result != 0) {                    // replace this
            if (now - attack_start_time > 1500 || attack_attempts >= 5) {
                printf("[Merlin] Attack completed - target eliminated (unit_attack result)\n");
                attack_active = false;        // NEW: mark attack over
                current_target = nullptr;     // NEW: clear local handle
                return false;
            }
        }                                     // with this (logic kept; only added end-of-fight flagging)
        
        printf("[Merlin] Attack executed (attempt %d)\n", attack_attempts);
    }
    
    // Throttled debug (kept)
    static uint64_t last_debug_log = 0;
    if (now - last_debug_log > 2000) {
        printf("[Debug] Attack in progress - attempt %d\n", attack_attempts);
        last_debug_log = now;
    }
    
    // Timeout fallback (kept)
    if (now - attack_start_time > 10000) {
        printf("[Merlin] Attack timeout - switching targets\n");
        attack_active = false;                // NEW: end the attack on timeout
        current_target = nullptr;             // NEW
        return false;
    }
    
    return true;
}

bool mln_attack_done(void) {
    if (!attack_active) {
        attack_start_time = 0;
        attack_attempts = 0;
        return true;
    }
    return false;
}
