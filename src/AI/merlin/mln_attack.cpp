#include "mln_attack.h"
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include <cstdio>

// Reference core globals  
extern int g_autoattack_account_id;      // Add this
extern GatMap* g_autoattack_map;         // Add this

// Attack state  
static bool attack_active = false;
static void* current_target = nullptr;  // nullptr is OK in C++
static int attack_counter = 0;
static uint64_t last_attack_time = 0;

extern const PluginContext* falconpm_get_context(void);

bool mln_attack_start(void* mob) {
    if (!mob) return false;
    
    current_target = mob;
    attack_active = true;
    attack_counter++;
    
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
    
    if (now - last_attack_time >= 500) {
        int result = ctx->combat->unit_attack(sd, (struct block_list*)current_target);
        last_attack_time = now;
        
        if (result != 0) {
            printf("[Merlin] Attack completed - target eliminated\n");
            return false;
        }
        
        printf("[Merlin] Attack executed\n");
    }
    
    return true;
}

bool mln_attack_done(void) {
    if (attack_active && !mln_attack_in_progress()) {
        attack_active = false;
        current_target = nullptr;
        return true;
    }
    return false;
}