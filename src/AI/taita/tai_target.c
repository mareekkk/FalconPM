#include "tai_api.h"
#include <stdio.h>

int tai_target_list(LootItem* out, int max_count) {
    // For now, just reuse loot list
    return tai_loot_list(out, max_count);
}

void tai_target_free(LootItem* arr) {
    tai_loot_free(arr);
}
