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
    MerlinState state = mln_api_get_state();

    switch (state) {
    case MLN_STATE_ROAMING: {
        const PluginContext* c = falconpm_get_context();
        if (c && c->log) {
            // Peregrine only provides pathfinding (astar, is_walkable, etc.)
            // It does not expose a high-level random roam like route_random().
            // For now, just log a stub so the state machine compiles.
            c->log->info("[Merlin] Roaming (stub) — no route_random/self available in PlayerAPI or PeregrineAPI.");
        }

        // TODO:
        // - Use PlayerAPI->map_id2sd(account_id) to resolve map_session_data*
        // - Use PeregrineAPI->astar() or is_walkable() to plan movement
        // - Replace this stub with real roaming logic
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
    
    case MLN_STATE_IDLE:
        // Handle idle state if needed
        break;
    }
}

#ifdef __cplusplus
}
#endif