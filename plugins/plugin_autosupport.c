#include "map/pc.h"
#include "map/status.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/unit.h"
#include "map/party.h"

extern void autosys_register(const char *name, void (*cb)(struct map_session_data*));
extern void autosys_delay(int,int);
extern bool autosys_skip(int);

#define MAX_AUTOSKILLS 5

static void autosupport_tick(struct map_session_data *sd) {
    int enabled = pc_readaccountreg(sd, script->add_str("#auto_support_enabled"));
    if (!enabled) return;
    autosys_delay(300, 800);

    for (int i = 1; i <= MAX_AUTOSKILLS; i++) {
        char key[32];
        sprintf(key, "#auto_supp%d_id", i);
        int skill_id = pc_readaccountreg(sd, script->add_str(key));
        sprintf(key, "#auto_supp%d_lv", i);
        int skill_lv = pc_readaccountreg(sd, script->add_str(key));
        sprintf(key, "#auto_supp%d_hp", i);
        int hp_th = pc_readaccountreg(sd, script->add_str(key));

        if (skill_id <= 0 || skill_lv <= 0) continue;
        if (pc_checkskill(sd, skill_id) < skill_lv) continue;

        if (hp_th == 0) {
            if (!status->get_status_icon(skill_id, sd)) {
                skill_castend_id(sd, skill_id, skill_lv, sd->bl.id, gettick(), 0);
                clif->message(sd->fd, "[AutoSys] Self buff cast.");
            }
            continue;
        }

        if (sd->status.party_id > 0) {
            struct party *p = party->search(sd->status.party_id);
            if (p) {
                for (int j = 0; j < MAX_PARTY; j++) {
                    struct map_session_data *psd = p->member[j].sd;
                    if (!psd || psd == sd) continue;
                    if (psd->bl.m != sd->bl.m) continue;
                    if (distance_bl(&sd->bl, &psd->bl) > 9) continue;
                    int hp_percent = psd->status.hp * 100 / psd->status.max_hp;
                    if (hp_percent < hp_th) {
                        if (autosys_skip(7)) return;
                        skill_castend_id(sd, skill_id, skill_lv, psd->bl.id, gettick(), 0);
                        clif->message(sd->fd, "[AutoSys] Support skill used on party.");
                        return;
                    }
                }
            }
        }
    }
}

HPExport void plugin_init(void) {
    autosys_register("autosupport", autosupport_tick);
}
