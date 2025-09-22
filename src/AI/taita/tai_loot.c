#include "tai_api.h"
#include <stdio.h>

// Dummy loot list for now
static LootItem dummy_loot[] = {
    {1, "Apple", 50, 50},
    {2, "Knife", 60, 42},
    {3, "Potion", 33, 70},
};

int tai_loot_list(LootItem* out, int max_count) {
    int count = sizeof(dummy_loot) / sizeof(dummy_loot[0]);
    if (count > max_count) count = max_count;
    for (int i = 0; i < count; i++) {
        out[i] = dummy_loot[i];
    }
    return count;
}

void tai_loot_free(LootItem* arr) {
    // nothing to free in dummy mode
    (void)arr;
}

void tai_loot_pickup(const LootItem* item) {
    if (!item) return;
    printf("[Taita] Picking up item: %s at (%d,%d)\n", item->name, item->x, item->y);
}
