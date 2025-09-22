#pragma once
#include "tai_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// For now just a placeholder interface
int tai_target_list(LootItem* out, int max_count);
void tai_target_free(LootItem* arr);

#ifdef __cplusplus
}
#endif
