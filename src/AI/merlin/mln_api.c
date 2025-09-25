#include "../../infra/plugin_api.h"
// #include "../../core/falconpm.hpp"   // COMMENTED: Include C++ header from C causes issues
#include "mln_api.h"
#include <stdio.h>
#include <stdint.h>

// ADDED: Forward declare C linkage function
#ifdef __cplusplus
extern "C" {
#endif

extern int g_autoattack_account_id;
extern const PluginContext* falconpm_get_context(void);  // ADDED: proper declaration

#ifdef __cplusplus
}
#endif

// Forward declare so compiler knows the type
struct map_session_data;

// Internal state tracker
static MerlinState current_state = MLN_STATE_ROAMING;

void mln_api_init(void) {
    current_state = MLN_STATE_ROAMING;
    printf("[Merlin] Initialized in ROAMING state\n");
}

void mln_api_set_state(MerlinState s) {
    current_state = s;
    switch (s) {
        case MLN_STATE_IDLE:       printf("[Merlin] State -> IDLE\n"); break;
        case MLN_STATE_ROAMING:    printf("[Merlin] State -> ROAMING\n"); break;
        case MLN_STATE_ATTACKING:  printf("[Merlin] State -> ATTACKING\n"); break;
        case MLN_STATE_WAIT_BUFFS: printf("[Merlin] State -> WAIT_BUFFS (buffing before attack)\n"); break; // [PATCH]
    }
}

MerlinState mln_api_get_state(void) {
    return current_state;
}

// Wrapper function for API compatibility
static void mln_api_set_state_wrapper(void* state) {
    mln_api_set_state((MerlinState)(intptr_t)state);
}

// ADDED: expose player session for Lanner
struct map_session_data* merlin_get_player(void) {
    const PluginContext* ctx = falconpm_get_context();
    if (!ctx || !ctx->player || g_autoattack_account_id < 0) {
        if (ctx && ctx->log) ctx->log->error("[Merlin] get_player: no valid account");
        return NULL;
    }
    return ctx->player->map_id2sd(g_autoattack_account_id);
}

// ADDED: Forward declare external functions
extern void merlin_tick(void);
extern void* mln_target_find(void);
extern bool mln_attack_start(void* mob);
extern bool mln_attack_in_progress(void);
extern bool mln_attack_done(void);

// API object - FIXED initialization with proper struct header
MerlinAPI merlin_api = {
    { sizeof(MerlinAPI), {1, 0} },           // ADDED: FpmTableHeader with size and version
    merlin_tick,                              // void (*tick)(void)
    mln_target_find,                          // void* (*target_find)(void)
    mln_attack_start,                         // bool (*attack_start)(void* mob)
    mln_attack_in_progress,                   // bool (*attack_in_progress)(void)
    mln_attack_done,                          // bool (*attack_done)(void)
    mln_api_set_state_wrapper,                // void (*set_state)(void* state)
    merlin_get_player                         // struct map_session_data* (*get_player)(void)
};
