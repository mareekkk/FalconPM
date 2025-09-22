#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Minimal target representation
typedef struct MobTarget {
    int id;
    int x;
    int y;
} MobTarget;

// Fill up to max_count targets, return count
int mln_target_list(MobTarget* out, int max_count);

// Free resources (no-op for now)
void mln_target_free(MobTarget* arr);

#ifdef __cplusplus
}
#endif
