#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------
// Loot item structure
// -----------------------------
typedef struct {
    int id;
    const char* name;
    int x;  // ✅ Added coordinate fields for compatibility with tai_loot.c
    int y;
} LootItem;

// -----------------------------
// Taita AI state
// -----------------------------
typedef enum {
    TAI_STATE_IDLE,
    TAI_STATE_SEARCHING,
    TAI_STATE_PICKING
} TaitaState;

// -----------------------------
// Prototypes
// -----------------------------
void tai_init(void);
void tai_tick(void);
void tai_set_state(TaitaState s);
TaitaState tai_get_state(void);

// Loot helpers (implemented in tai_loot.c / tai_target.c)
int tai_loot_list(LootItem* out, int max_count);
int tai_target_list(LootItem* out, int max_count);

// Aliases for FalconPM API table (implemented in tai_api.c)
LootItem* tai_target_find_items(int* count);
void      tai_loot_pickup(const LootItem* item);

// ------------------------------------------------------
// ❌ OLD (from your plugin_api.h version)
// typedef struct TaitaAPI {
//     void*   (*find_items)();
//     void    (*free_items)(void* ptr);
//     const char* (*get_item)(void* ptr, size_t idx);
//     size_t  (*get_item_count)(void* ptr);
//     void    (*loot_pickup)(const char* item);
//     void    (*tick)();  
// } TaitaAPI;
// ------------------------------------------------------

// ------------------------------------------------------
// ✅ NEW: Simplified ABI-safe typedef
//    Using void* for opaque LootItem* to avoid type conflicts.
// ------------------------------------------------------
typedef struct TaitaAPI {
    int   (*target_find_items)(void* out, int max_count); // LootItem* passed as void*
    void  (*loot_pickup)(const void* item);               // const LootItem* as void*
    void  (*tick)(void);                                  // orchestrator loop
} TaitaAPI;

// Global instance (defined in tai_api.c)
extern TaitaAPI taita_api;

#ifdef __cplusplus
}
#endif
