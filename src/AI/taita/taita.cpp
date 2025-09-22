#include "tai_api.h"
#include <stdio.h>

// Internal state
static TaitaState state = TAI_STATE_IDLE;

void tai_init(void) {
    state = TAI_STATE_IDLE;
}

void tai_set_state(TaitaState s) {
    state = s;
}

TaitaState tai_get_state(void) {
    return state;
}

void tai_tick(void) {
    switch (state) {
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
