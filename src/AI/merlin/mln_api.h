#pragma once
#include <stdbool.h>
#include <stdint.h>  // ADD this line for intptr_t
#include "../../infra/plugin_api.h"
#include "merlin.h"

#ifdef __cplusplus
extern "C" {
#endif

// Add this function declaration:
void mln_api_set_state(MerlinState s);

// Forward declarations
struct map_session_data;
typedef struct MobTarget MobTarget;

// --- State machine API ---
void mln_api_init(void);                 // Initialize Merlin state
void mln_api_set_state(MerlinState s);   // Set state
MerlinState mln_api_get_state(void);     // Get current state

// --- Attack + Target functions ---
bool mln_attack_start(void* mob);
bool mln_attack_in_progress(void);
bool mln_attack_done(void);
void* mln_target_find(void);

// Orchestrator tick
void merlin_tick(void);

// Global instance (defined in mln_api.c)
extern MerlinAPI merlin_api;

#ifdef __cplusplus
}
#endif
