#include "../../infra/plugin_api.h"
#include "../../AI/peregrine/peregrine.h"

static const PluginContext* g_ctx = nullptr;
static int g_account_id = -1;

// timer tick
static int aa_tick(int tid, uint64_t tick, int id, intptr_t data) {
    if (g_account_id < 0) return 0;

    map_session_data* sd = g_ctx->player->map_id2sd(g_account_id);
    if (!sd) return 0;  // player logged out

    block_list* mob = g_ctx->combat->get_nearest_mob(sd, 15);
    if (mob) {
        g_ctx->combat->unit_attack(sd, mob);
    }

    g_ctx->timer->add_timer(tick + 200, aa_tick, 0, 0);
    return 0;
}

// @aa command
static int cmd_autoattack(map_session_data* sd, const char* cmd, const char* msg) {
    g_account_id = g_ctx->player->get_account_id(sd);
    g_ctx->log->info("[autoattack] enabled for account %d", g_account_id);
    g_ctx->timer->add_timer(g_ctx->timer->gettick() + 100, aa_tick, 0, 0);
    g_ctx->player->send_message(sd, "[autoattack] command triggered");

    return 0;
}

// plugin entry/exit
extern "C" {

bool plugin_init(const PluginContext* ctx) {
    g_ctx = ctx;
    g_ctx->log->info("[autoattack] init OK (@aa)");
    ctx->atcommand->add("aa", cmd_autoattack);
    return true;
}

void plugin_final() {
    // cleanup if needed
}

PluginDescriptor PLUGIN = {
    "autoattack",
    "0.1",
    nullptr,
    plugin_init,
    plugin_final
};

}
