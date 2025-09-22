#pragma once
#include <stddef.h>   // for size_t
#include <stdbool.h>  // for bool

#ifdef __cplusplus
#include <string>     // if you want item names as std::string
#include <vector>     // if you want dynamic lists
#endif
// --------------------------
// Core Loot Item definition
// --------------------------
typedef struct LootItem {
    int id;              // Item ID
    char name[64];       // Item name (fixed buffer for now)
    int x, y;            // Map position
} LootItem;

// --------------------------
// API for Taita subsystem
// --------------------------
#ifdef __cplusplus
extern "C" {
#endif

// State machine for looting
typedef enum {
    TAI_STATE_IDLE,
    TAI_STATE_SEARCHING,
    TAI_STATE_PICKING
} TaitaState;

// Simple stub implementations to satisfy FalconPM linking
LootItem* tai_target_find_items(int* count) {
    static LootItem dummy[1];
    *count = tai_target_list(dummy, 1);
    return (count > 0) ? dummy : NULL;
}

void tai_loot_pickup(const LootItem* item) {
    // For now, just forward to existing function
    // (Assumes tai_loot_pickup from header is implemented elsewhere or expand here)
}

// Prototypes
void tai_init(void);
void tai_tick(void);
void tai_set_state(TaitaState s);
TaitaState tai_get_state(void);

// Aliases for FalconPM API table
LootItem* tai_target_find_items(int* count);  // returns array pointer + count

// Loot handling
int  tai_loot_list(LootItem* out, int max_count);
void tai_loot_free(LootItem* arr);
void tai_loot_pickup(const LootItem* item);

// Target helpers
int  tai_target_list(LootItem* out, int max_count);
void tai_target_free(LootItem* arr);

#ifdef __cplusplus
}
#endif
