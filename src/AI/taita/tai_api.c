#include "tai_api.h"

// Current loot state
static TaitaState current_state = TAI_STATE_IDLE;

TaitaState tai_api_get_state(void) {
    return current_state;
}

void tai_api_set_state(TaitaState s) {
    current_state = s;
}
