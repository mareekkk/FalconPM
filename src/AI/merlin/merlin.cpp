#include "mln_api.h"
#include "mln_target.h"
#include "mln_attack.h"
#include <cstdio>
#include <vector>
#include "../../infra/plugin_api.h"
//#include "../../core/falconpm.cpp"
#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
extern const PluginContext* falconpm_get_context(void);

// merlin_tick must be inside extern "C" for C linkage
void merlin_tick() {
    static MerlinState last_state = MLN_STATE_IDLE; // remember last state
    MerlinState state = mln_api_get_state();
    const PluginContext* c = falconpm_get_context();

    // ----------------------------------------------------
    // Guard: skip if no player session (e.g. char select)
    // ----------------------------------------------------
    if (!c || !c->player) return;

    // If you track account_id, resolve sd here:
    // map_session_data* sd = c->player->map_id2sd(active_account_id);
    // if (!sd) return;

    switch (state) {
    case MLN_STATE_ROAMING: {
        if (state != last_state && c->log) {
            c->log->info("[Merlin] Entering ROAMING state (stub) — no route_random/self available yet.");
        }
        // TODO:
        // - Use PlayerAPI->map_id2sd(account_id) to resolve map_session_data*
        // - Use PeregrineAPI->astar() or is_walkable() to plan movement
        // - Replace this stub with real roaming logic
        break;
    }

    case MLN_STATE_ATTACKING: {
        if (mln_attack_in_progress()) {
            if (state != last_state && c->log) {
                c->log->info("[Merlin] Entering ATTACKING state");
            }
            printf("[Merlin] Attacking...\n");
        } else if (mln_attack_done()) {
            if (c->log) c->log->info("[Merlin] Monster killed, returning to ROAMING");
            mln_api_set_state(MLN_STATE_ROAMING);
        }
        break;
    }

    case MLN_STATE_IDLE: {
        if (state != last_state && c->log) {
            c->log->info("[Merlin] Entering IDLE state");
        }
        break;
    }
    }

    last_state = state; // update tracker
}


#ifdef __cplusplus
}
#endif