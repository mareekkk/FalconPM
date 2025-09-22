#pragma once
#include "tai_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Loot handling functions
int tai_loot_list(LootItem* out, int max_count);
void tai_loot_free(LootItem* arr);
void tai_pickup(const char* item);

#ifdef __cplusplus
}
#endif
