#include "mln_api.h"
#include <stdio.h>

// Internal state tracker
static MerlinState current_state = MLN_STATE_ROAMING;

void mln_api_init(void) {
    current_state = MLN_STATE_ROAMING;
    printf("[Merlin] Initialized in ROAMING state\n");
}

void mln_api_set_state(MerlinState s) {
    current_state = s;
    switch (s) {
        case MLN_STATE_IDLE:      printf("[Merlin] State -> IDLE\n"); break;
        case MLN_STATE_ROAMING:   printf("[Merlin] State -> ROAMING\n"); break;
        case MLN_STATE_ATTACKING: printf("[Merlin] State -> ATTACKING\n"); break;
    }
}

MerlinState mln_api_get_state(void) {
    return current_state;
}

// ------------------------------------
// Merlin API object
// ------------------------------------
MerlinAPI merlin_api = {
    (void (*)(void))merlin_tick,              // matches void()
    (void* (*)(void))mln_target_find,         // cast MobTarget* -> void*
    (bool (*)(void*))mln_attack_start,        // cast MobTarget* -> void*
    (bool (*)(void))mln_attack_in_progress,   // matches bool()
    (bool (*)(void))mln_attack_done           // matches bool()
};