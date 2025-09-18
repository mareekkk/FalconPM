#include "map/pc.h"
#include "map/status.h"
#include "map/script.h"

extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
extern void falconpm_delay(int,int);
extern bool falconpm_skip(int);
extern int falconpm_jitter(int,int);

static void autopots_tick(struct map_session_data *sd) {
    int hp_th = pc_readaccountreg(sd, script->add_str("#auto_hp_threshold"));
    int hp_item = pc_readaccountreg(sd, script->add_str("#auto_hp_item"));

    if (hp_th <= 0 || hp_item <= 0) return;

    // Context awareness: only run if alive and not sitting
    if (pc_isdead(sd) || pc_issit(sd)) return;

    // Humanized threshold (jitter)
    int hp_percent = sd->status.hp * 100 / sd->status.max_hp;
    hp_th = falconpm_jitter(hp_th, 3);

    if (hp_percent < hp_th) {
        // Humanization: chance to hesitate
        if (falconpm_skip(5)) return;

        // Humanization: reaction delay before using potion
        falconpm_delay(200, 600);

        pc_useitem(sd, hp_item);
        clif->message(sd->fd, "[FalconPM] Auto-potion (humanized).");
    }
}

HPExport void plugin_init(void) {
    falconpm_register("autopots", autopots_tick);
}
