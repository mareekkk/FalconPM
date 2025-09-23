// src/core/falconpm.cpp
// FalconPM base module: provides API tables to plugins

#include "falconpm.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <unordered_map>
#include <string>

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
    fpm_get_nearest_mob,
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
