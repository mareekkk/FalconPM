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

// ------------------------------------------------------
// Attack + Target functions
// ------------------------------------------------------
// Begin an attack on the given target
bool mln_attack_start(MobTarget* t);

// Return true if an attack is still ongoing
bool mln_attack_in_progress(void);

// Return true if the current attack has finished
bool mln_attack_done(void);

// Find nearest mob target (returns nullptr if none)
MobTarget* mln_target_find(void);

// ------------------------------------------------------
// Merlin API table definition
// ------------------------------------------------------
// This struct is what FalconPM core binds into ctx->merlin
typedef struct MerlinAPI {
    void (*tick)(void);                  // AI tick handler
    MobTarget* (*target_find)(void);     // target acquisition
    bool (*attack_start)(MobTarget*);    // attack initiation
    bool (*attack_in_progress)(void);    // attack progress check
    bool (*attack_done)(void);           // attack completion check
} MerlinAPI;

// ------------------------------------------------------
// Global instance (defined in mln_api.c)
// ------------------------------------------------------
extern MerlinAPI merlin_api;

#ifdef __cplusplus
}
#endif
