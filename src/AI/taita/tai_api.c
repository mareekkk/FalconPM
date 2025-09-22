#include "tai_api.h"
#include <stdio.h>

// -----------------------------
// Current loot state
// -----------------------------
static TaitaState current_state = TAI_STATE_IDLE;

// -----------------------------
// State management
// -----------------------------
TaitaState tai_get_state(void) {
    return current_state;
}

void tai_set_state(TaitaState s) {
    current_state = s;
}

// -----------------------------
// Taita tick loop
// -----------------------------
void tai_tick(void) {
    switch (current_state) {
        case TAI_STATE_IDLE:
            printf("[Taita] Idle, switching to searching\n");
            tai_set_state(TAI_STATE_SEARCHING);
            break;

        case TAI_STATE_SEARCHING: {
            LootItem items[10];
            int count = tai_loot_list(items, 10);
            if (count > 0) {
                printf("[Taita] Found %d items, switching to picking\n", count);
                tai_set_state(TAI_STATE_PICKING);
                tai_loot_pickup(&items[0]);
            } else {
                printf("[Taita] No items found.\n");
            }
            break;
        }

        case TAI_STATE_PICKING:
            printf("[Taita] Currently picking.\n");
            tai_set_state(TAI_STATE_IDLE);
            break;
    }
}

// -----------------------------
// FalconPM-facing wrappers
// -----------------------------
LootItem* tai_target_find_items(int* count) {
    static LootItem buffer[16];
    *count = tai_target_list(buffer, 16);
    return (*count > 0) ? buffer : NULL;
}

void tai_loot_pickup(const LootItem* item) {
    if (!item) return;
    printf("[Taita] Looting item: %s (ID=%d)\n",
           item->name ? item->name : "unknown",
           item->id);
    // TODO: forward to bootstrap pc_takeitem wrapper
}
