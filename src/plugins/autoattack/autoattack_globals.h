#pragma once
#include "../../AI/peregrine/pgn_gat.h"

#ifdef __cplusplus
extern "C" {
#endif

// Globals exported by AutoAttack for Merlin to use
extern int g_autoattack_account_id;
extern GatMap* g_autoattack_map;

#ifdef __cplusplus
}
#endif