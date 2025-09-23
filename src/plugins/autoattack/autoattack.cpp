// src/plugins/autoattack/autoattack.cpp
// FalconPM AutoAttack Plugin

#include "../../infra/plugin_api.h"
#include "../../AI/peregrine/peregrine.h"
#include "../../AI/peregrine/pgn_path.h"
#include "../../core/falconpm.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

static const PluginContext* ctx = nullptr;
static bool g_enabled=false;
static int g_account_id=-1;
static block_list* g_target=nullptr;
static GatMap* g_map=nullptr;
static uint64_t g_last_attack=0;

enum AttackState { SEARCHING, ATTACKING };
static AttackState state=SEARCHING;

static GatMap* load_gat(map_session_data* sd) {
    int m=fpm_get_sd_m(sd);
    const char* name=fpm_get_map_name(m);
    if (!name) return nullptr;
    char fn[512]; snprintf(fn,sizeof(fn),FALCONPM_GAT_PATH "%s.gat",name);
    return ctx->peregrine->load_gat(fn);
}

static bool move_to(map_session_data* sd, block_list* target) {
    if (!sd||!target||!g_map) return false;
    int sx=fpm_get_sd_x(sd), sy=fpm_get_sd_y(sd);
    int tx=fpm_get_bl_x(target), ty=fpm_get_bl_y(target);
    PStepList steps;
    if (!ctx->peregrine->astar(g_map,sx,sy,tx,ty,&steps)||steps.count<=0) return false;
    ctx->peregrine->route_start(ctx,sd,&steps,g_map);
    return true;
}

static int aa_tick(int tid,uint64_t tick,int id,intptr_t data) {
    if (!g_enabled||g_account_id<0) return 0;
    map_session_data* sd=ctx->player->map_id2sd(g_account_id);
    if (!sd) return 0;

    switch(state){
        case SEARCHING:{
            block_list* mob=ctx->combat->get_nearest_mob(sd,15);
            if(mob){ g_target=mob;
                if(move_to(sd,mob)) state=ATTACKING;
            }
        } break;
        case ATTACKING:{
            if(!g_target){state=SEARCHING;break;}
            if(tick-g_last_attack>=500){
                int r=ctx->combat->unit_attack(sd,g_target);
                g_last_attack=tick;
                if(r!=0){ g_target=nullptr; state=SEARCHING; }
            }
        } break;
    }
    ctx->timer->add_timer(tick+200,aa_tick,0,0);
    return 0;
}

static int cmd_aa(map_session_data* sd,const char* c,const char* m){
    if(!sd)return-1;
    if(m&&strcmp(m,"off")==0){ g_enabled=false; ctx->peregrine->route_stop(); return 0;}
    if(!g_enabled){ g_account_id=ctx->player->get_account_id(sd);
        g_map=load_gat(sd); g_enabled=true; state=SEARCHING;
        ctx->timer->add_timer(ctx->timer->gettick()+100,aa_tick,0,0);}
    else { g_enabled=false; ctx->peregrine->route_stop(); }
    return 0;
}

extern "C" {
bool plugin_init(const PluginContext* u){(void)u;ctx=falconpm_get_context();
 ctx->atcommand->add("aa",cmd_aa);return true;}
void plugin_final(){g_enabled=false;ctx->peregrine->route_stop();}
PluginDescriptor PLUGIN={"autoattack","0.3",nullptr,plugin_init,plugin_final};
}
