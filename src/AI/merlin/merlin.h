#pragma once
#include <stdbool.h>   // for bool

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------
// Forward declarations
// ------------------------------------------------------
struct map_session_data;   // player session (from bootstrap)
typedef struct MobTarget MobTarget;  // opaque mob target type

// ------------------------------------------------------
// Merlin AI states
// ------------------------------------------------------
typedef enum {
    MLN_STATE_IDLE,       // doing nothing
    MLN_STATE_ROAMING,    // moving around looking for mobs
    MLN_STATE_ATTACKING   // actively attacking
} MerlinState;

// ------------------------------------------------------
// State machine API
// ------------------------------------------------------
// Initialize Merlin state machine
void mln_api_init(void);

// Force-set the current Merlin state
void mln_api_set_state(MerlinState s);

// Query current Merlin state
MerlinState mln_api_get_state(void);

// --- Attack + Target functions ---
bool mln_attack_start(void* mob);           // Changed from MobTarget* to void*
bool mln_attack_in_progress(void);
bool mln_attack_done(void);
void* mln_target_find(void);                // Changed from MobTarget* to void*

// ------------------------------------------------------
// Global instance (defined in mln_api.c)
// ------------------------------------------------------
extern MerlinAPI merlin_api;

#ifdef __cplusplus
}
#endif
