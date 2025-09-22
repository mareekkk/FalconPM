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

// Prototypes
void tai_init(void);
void tai_tick(void);
void tai_set_state(TaitaState s);
TaitaState tai_get_state(void);

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
