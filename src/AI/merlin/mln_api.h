#pragma once
#include <stdbool.h>   // for bool
#include "../../infra/plugin_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
struct map_session_data;
typedef struct MobTarget MobTarget;

// Merlin AI states
typedef enum {
    MLN_STATE_IDLE,
    MLN_STATE_ROAMING,
    MLN_STATE_ATTACKING
} MerlinState;

// --- State machine API ---
void mln_api_init(void);                 // Initialize Merlin state
void mln_api_set_state(MerlinState s);   // Set state
MerlinState mln_api_get_state(void);     // Get current state

// --- Attack + Target functions ---
bool mln_attack_start(MobTarget* t); 
bool mln_attack_in_progress(void);
bool mln_attack_done(void);
MobTarget* mln_target_find(void);

// Global instance (defined in mln_api.c)
extern MerlinAPI merlin_api;

#ifdef __cplusplus
}
#endif
