#include "mln_api.h"
#include "mln_target.h"
#include "mln_attack.h"
#include <cstdio>
#include <vector>
#include "../../infra/plugin_api.h"

extern PluginContext g_ctx;


// External: autoroute random walk
extern void autoroute_random_step();

void merlin_tick() {
    MerlinState state = mln_api_get_state();

    switch (state) {
    case MLN_STATE_ROAMING: {
        MobTarget target;
        int count = mln_target_list(&target, 1);  // ask for 1 mob
        if (count > 0) {
            printf("[Merlin] Monster found, engaging\n");
            mln_attack_start(&target);  // fixed signature
            mln_api_set_state(MLN_STATE_ATTACKING);
        } else {
            printf("[Merlin] No monster found, roaming...\n");
            autoroute_random_step();
        }
        break;
    }

    case MLN_STATE_ATTACKING: {
        if (mln_attack_in_progress()) {
            printf("[Merlin] Attacking...\n");
        } else if (mln_attack_done()) {
            printf("[Merlin] Monster killed, returning to roam\n");
            mln_api_set_state(MLN_STATE_ROAMING);
        }
        break;
    }
    }
}
