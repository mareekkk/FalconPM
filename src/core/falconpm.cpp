// src/core/falconpm.cpp
// FalconPM base module: provides API tables to plugins

#include "falconpm.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <unordered_map>
#include <string>
#include "../AI/peregrine/pgn_gat.h"

extern "C" {
    void exported_logic_function() {
        // Export business logic
    }
}

// ----------------------------------------------------
// Logging
// ----------------------------------------------------
static void log_info_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}
static void log_error_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// ----------------------------------------------------
// Shared globals for Merlin combat system
// ----------------------------------------------------
int g_autoattack_account_id = -1;        // Add this
GatMap* g_autoattack_map = nullptr;      // Add this

// ----------------------------------------------------
// Bootstrap function declarations (C linkage)
// ----------------------------------------------------
extern "C" {
    int fpm_count_players_near_position(int x, int y, int exclude_account_id, int range);
    int fpm_get_bl_id(block_list* bl);
    int fpm_get_bl_x(block_list* bl);  
    int fpm_get_bl_y(block_list* bl);
    int fpm_get_account_id(map_session_data* sd);
    bool fpm_is_mob_alive(block_list* mob);           
    int fpm_get_mob_hp_percent(block_list* mob);      
    bool fpm_is_mob_in_combat(block_list* mob);
}

// ----------------------------------------------------
// Anti-KS tracking (processed data approach)  
// ----------------------------------------------------
static std::unordered_map<int, uint64_t> engaged_mobs;
static std::unordered_map<int, int> mob_attackers;
static const int ANTI_KS_RANGE = 5;
static const int ENGAGEMENT_TIMEOUT = 10000;

// Internal C++ helper function (unchanged)
// Enhanced mob availability check with vitality validation
// Enhanced mob availability check with vitality validation
static bool is_mob_available(int mob_id, int mob_x, int mob_y, int requesting_account_id, block_list* mob_ptr) {
    // Use bootstrap functions for rAthena struct access
    if (!fpm_is_mob_alive(mob_ptr)) {
        return false;
    }
    
    int hp_percent = fpm_get_mob_hp_percent(mob_ptr);
    if (hp_percent < 10) {
        return false;
    }
    
    if (fpm_is_mob_in_combat(mob_ptr)) {
        return false;
    }
    
    // Rest of function unchanged...
    uint64_t now = fpm_gettick();
    
    auto it = engaged_mobs.find(mob_id);
    if (it != engaged_mobs.end()) {
        if (now - it->second < ENGAGEMENT_TIMEOUT) {
            auto attacker_it = mob_attackers.find(mob_id);
            if (attacker_it != mob_attackers.end() && 
                attacker_it->second != requesting_account_id) {
                return false;
            }
        } else {
            engaged_mobs.erase(it);
            mob_attackers.erase(mob_id);
        }
    }
    
    int nearby_players = fpm_count_players_near_position(mob_x, mob_y, requesting_account_id, ANTI_KS_RANGE);
    
    return (nearby_players == 0);
}

// Export functions with C linkage (unchanged)
extern "C" {
    void mark_mob_engaged(int mob_id, int account_id) {
        engaged_mobs[mob_id] = fpm_gettick();
        mob_attackers[mob_id] = account_id;
    }

    void clear_mob_engagement(int mob_id) {
        engaged_mobs.erase(mob_id);
        mob_attackers.erase(mob_id);
    }

    bool is_mob_engaged_by_other(int mob_id, int account_id) {
        auto attacker_it = mob_attackers.find(mob_id);
        if (attacker_it != mob_attackers.end()) {
            return attacker_it->second != account_id;
        }
        return false;
    }
}

// ----------------------------------------------------
// Dummy stubs
// ----------------------------------------------------
static struct block_list* dummy_get_target(void* u) { (void)u; return nullptr; }
static int dummy_get_id(struct block_list* bl) { (void)bl; return 0; }
static int dummy_get_type(struct block_list* bl) { (void)bl; return 0; }
static int32_t rnd_impl(void) { return rand(); }

// ----------------------------------------------------
// Atcommand support
// ----------------------------------------------------
static std::unordered_map<std::string, AtCmdFunc> fpm_atcmds;
using is_atcommand_t = bool(*)(const int32_t, map_session_data*, const char*, int32_t);
static is_atcommand_t orig_is_atcommand = nullptr;

bool is_atcommand(const int32_t fd, map_session_data* sd, const char* message, int32_t type) {
    if (message && (message[0] == '@' || message[0] == '#')) {
        std::string cmd = message + 1;
        auto it = fpm_atcmds.find(cmd);
        if (it != fpm_atcmds.end()) {
            it->second(sd, cmd.c_str(), "");
            return true;
        }
    }
    if (orig_is_atcommand) return orig_is_atcommand(fd, sd, message, type);
    return false;
}

static bool at_add_wrapper(const char* name, AtCmdFunc func) {
    if (!name || !func) return false;
    fpm_atcommand_register(name, func);
    return true;
}
static bool at_remove_wrapper(const char* name) {
    if (!name) return false;
    return fpm_atcommand_unregister && fpm_atcommand_unregister(name);
}

// Enhanced get_nearest_mob with vitality and anti-KS validation
static block_list* fpm_get_nearest_mob_antiks(map_session_data* sd, int range) {
    if (!sd) return nullptr;
    
    block_list* nearest = fpm_get_nearest_mob(sd, range);
    
    // Enhanced validation with vitality check
    if (nearest) {
        int mob_id = fpm_get_bl_id(nearest);
        int mob_x = fpm_get_bl_x(nearest);
        int mob_y = fpm_get_bl_y(nearest);
        int account_id = fpm_get_account_id(sd);
        
        if (!is_mob_available(mob_id, mob_x, mob_y, account_id, nearest)) {
            return nullptr; // Mob failed vitality or anti-KS checks
        }
    }
    
    return nearest;
}

// ----------------------------------------------------
// API tables
// ----------------------------------------------------
static LogAPI log_api = {
    { sizeof(LogAPI), {1,0} },
    log_info_impl,
    log_error_impl
};
static UnitAPI unit_api = {
    { sizeof(UnitAPI), {1,0} },
    dummy_get_target,
    dummy_get_id,
    dummy_get_type
};
static PlayerAPI player_api = {
    { sizeof(PlayerAPI), {1,0} },
    fpm_map_id2sd,
    fpm_send_message,
    fpm_get_account_id
};
static RandomAPI rnd_api = {
    { sizeof(RandomAPI), {1,0} },
    rnd_impl
};
static AtcommandAPI atcommand_api = {
    { sizeof(AtcommandAPI), {1,0} },
    at_add_wrapper,
    at_remove_wrapper
};
static PlayerMovementAPI movement_api = {
    { sizeof(PlayerMovementAPI), {1,0} },
    fpm_pc_walktoxy,
    fpm_unit_walktoxy
};
static PathAPI path_api = {
    { sizeof(PathAPI), {1,0} },
    fpm_path_search
};
static DirectionAPI dir_api = {
    { sizeof(DirectionAPI), {1,0} },
    fpm_get_dirx(),
    fpm_get_diry()
};
static TimerAPI timer_api = {
    { sizeof(TimerAPI), {1,0} },
    fpm_add_timer,
    fpm_gettick
};
static CombatAPI combat_api = {
    { sizeof(CombatAPI), {1,0} },
    fpm_get_nearest_mob_antiks,  // Use enhanced version
    fpm_unit_attack
};

// ----------------------------------------------------
// Extern API objects
// ----------------------------------------------------
extern "C" PeregrineAPI peregrine_api;
extern "C" MerlinAPI merlin_api;

// ----------------------------------------------------
// Global g_ctx (exported)
// ----------------------------------------------------
PluginContext g_ctx = {
    {1,0},
    &log_api,
    &unit_api,
    &player_api,
    &rnd_api,
    &atcommand_api,
    &movement_api,
    &timer_api,
    &path_api,
    &dir_api,
    &peregrine_api,
    &combat_api,
    &merlin_api
};

// Accessor for plugins
extern "C" const PluginContext* falconpm_get_context(void) {
    return &g_ctx;
}

// ----------------------------------------------------
// AI runner
// ----------------------------------------------------
static int falconpm_ai_runner(int tid, uint64_t tick, int id, intptr_t data) {
    (void)tid; (void)id; (void)data;

    // Combat AI
    if (g_ctx.merlin && g_ctx.merlin->tick)
        g_ctx.merlin->tick();

    // Loot AI
    // if (g_ctx.taita && g_ctx.taita->tick)
    //     g_ctx.taita->tick();

    // Navigation AI
    if (g_ctx.peregrine && g_ctx.peregrine->tick)
        g_ctx.peregrine->tick();

    // Reschedule every 100ms
    fpm_add_timer(fpm_gettick() + 100, falconpm_ai_runner, 0, 0);
    return 0;
}


// ----------------------------------------------------
// Plugin descriptor
// ----------------------------------------------------
static const int* required_modules(size_t* count) {
    *count = 0;
    return nullptr;
}

static bool init(const PluginContext* c) {
    (void)c;
    mln_api_init();
    fpm_add_timer(fpm_gettick() + 100, falconpm_ai_runner, 0, 0);
    g_ctx.log->info("FalconPM core initialized.");
    return true;
}

static void shutdown(void) {
    g_ctx.log->info("FalconPM core shutting down.");
}

extern "C" {
PluginDescriptor PLUGIN = {
    "falconpm_base",
    "0.3",
    required_modules,
    init,
    shutdown
};
}
