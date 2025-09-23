#pragma once

#include "../../infra/plugin_api.h"

#ifdef __cplusplus
extern "C" {
#endif

bool mln_attack_start(void* mob);
bool mln_attack_in_progress(void);
bool mln_attack_done(void);

#ifdef __cplusplus
}
#endif
