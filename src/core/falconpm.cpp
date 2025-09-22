// falconpm.cpp
// FalconPM base module: provides API tables to plugins

#include "../infra/plugin_api.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <unordered_map>
#include <string>
#include "pc.hpp"
#include "unit.hpp"
#include "atcommand.hpp"

#include "../AI/merlin/mln_api.h"
#include "../AI/merlin/mln_target.h"
#include "../AI/merlin/mln_attack.h"
#include "../AI/merlin/merlin.cpp"

#include "../AI/taita/tai_api.h"
#include <stdint.h>

// ----------------------------------------------------
// Global context
// ----------------------------------------------------
static const PluginContext* ctx = nullptr;
extern "C" PeregrineAPI peregrine_api;

// Forward declare API objects
extern MerlinAPI merlin_api;
extern TaitaAPI taita_api;

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
// AI runner
// ----------------------------------------------------
static int falconpm_ai_runner(int tid, uint64_t tick, int id, intptr_t data) {
    (void)tid; (void)id; (void)data;

    if (ctx && ctx->merlin && ctx->merlin->tick)
        ctx->merlin->tick();
    if (ctx && ctx->taita && ctx->taita->tick)
        ctx->taita->tick();

    // reschedule runner every 100ms
    fpm_add_timer(fpm_gettick() + 100, falconpm_ai_runner, 0, 0);
    return 0;
}

// ----------------------------------------------------
// rAthena bridge externs
// ----------------------------------------------------
extern "C" {
    int fpm_get_sd_x(map_session_data* sd);
    int fpm_get_sd_y(map_session_data* sd);
    int fpm_get_sd_m(map_session_data* sd);

    int fpm_get_bl_x(block_list* bl);
    int fpm_get_bl_y(block_list* bl);
    int fpm_get_bl_id(block_list* bl);
}

// combat
extern "C" {
    block_list* fpm_get_nearest_mob(map_session_data* sd, int range);
    int fpm_unit_attack(map_session_data* sd, block_list* target);
}
static CombatAPI combat_api = {
    { sizeof(CombatAPI), {1,0} },
    fpm_get_nearest_mob,
    fpm_unit_attack
};

extern "C" void fpm_send_message(map_session_data* sd, const char* msg);
extern "C" int fpm_get_account_id(map_session_data* sd);

// ----------------------------------------------------
// Timer
// ----------------------------------------------------
extern "C" {
    int fpm_add_timer(uint64_t tick, int (*func)(int, uint64_t, int, intptr_t), int id, intptr_t data);
    uint64_t fpm_gettick(void);
}
static TimerAPI timer_api = {
    { sizeof(TimerAPI), {1,0} },
    fpm_add_timer,
    fpm_gettick
};

extern "C" {
    map_session_data* fpm_map_id2sd(int account_id);
}

// ----------------------------------------------------
// Taita + Merlin APIs
// ----------------------------------------------------
static TaitaAPI taita_api = {
    tai_target_find_items,
    tai_loot_pickup,
    taita_tick
};

static MerlinAPI merlin_api = {
    merlin_tick,
    mln_target_find,
    mln_attack_start,
    mln_attack_in_progress,
    mln_attack_done
};

// ----------------------------------------------------
// Dummy stubs
// ----------------------------------------------------
static struct block_list* dummy_get_target(void* u) { (void)u; return nullptr; }
static int dummy_get_id(struct block_list* bl) { (void)bl; return 0; }
static int dummy_get_type(struct block_list* bl) { (void)bl; return 0; }
static struct map_session_data* dummy_map_id2sd(int aid) { (void)aid; return nullptr; }
static void dummy_send_message(int fd, const char* msg) { (void)fd; printf("[MSG] %s\n", msg); }
static int32_t rnd_impl(void) { return rand(); }

// ----------------------------------------------------
// Atcommand
// ----------------------------------------------------
static std::unordered_map<std::string, AtCmdFunc> fpm_atcmds;

using is_atcommand_t = bool(*)(const int32, map_session_data*, const char*, int32);
static is_atcommand_t orig_is_atcommand = nullptr;

bool is_atcommand(const int32 fd, map_session_data* sd, const char* message, int32 type) {
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

extern "C" bool fpm_atcommand_register(const char* name, AtCmdFunc func);
extern "C" bool fpm_atcommand_unregister(const char* name);

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

// From bootstrap
extern "C" {
    int fpm_pc_walktoxy(map_session_data* sd, short x, short y, int type);
    int fpm_unit_walktoxy(block_list* bl, short x, short y, unsigned char flag);
    int fpm_path_search(struct walkpath_data *wpd, int m,
                        int x0, int y0, int x1, int y1, int flag);
    const int16_t* fpm_get_dirx();
    const int16_t* fpm_get_diry();
}

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
    &merlin_api,
    &taita_api
};

// ----------------------------------------------------
// Plugin descriptor
// ----------------------------------------------------
static const int* required_modules(size_t* count) {
    *count = 0;
    return nullptr;
}

static bool init(PluginContext* c) {
    ctx = c;
    ctx->merlin = &merlin_api;
    ctx->taita  = &taita_api;

    fpm_add_timer(fpm_gettick() + 100, falconpm_ai_runner, 0, 0);

    ctx->log->info("FalconPM core initialized.");
    return true;
}

static void final(void) {
    ctx->log->info("FalconPM core shutting down.");
}

extern "C" {
PluginDescriptor PLUGIN = {
    "falconpm_base",
    "0.3",
    required_modules,
    init,
    final
};

const PluginContext* falconpm_get_context(void) { return &g_ctx; }
}
