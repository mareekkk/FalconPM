// /mnt/data/mln_attack.h
// Full replacement - attack API header (C-compatible)

#pragma once

#include "../../infra/plugin_api.h"
#include "mln_target.h"

// Forward declare MobTarget
typedef struct MobTarget MobTarget;

#ifdef __cplusplus
extern "C" {
#endif

/* Start an attack on the chosen target (no rAthena struct exposure here). */
bool mln_attack_start(void* mob);

/* True while we consider ourselves attacking. */
bool mln_attack_in_progress(void);

/* Becomes true once attack is finished; resets internal flag. */
bool mln_attack_done(void);

#ifdef __cplusplus
}
#endif
