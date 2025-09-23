// src/core/falconpm.hpp
#pragma once
#include "../infra/plugin_api.h"
#include <stdint.h>

// Forward declarations
struct map_session_data;
struct block_list;
struct walkpath_data;

// Access global context
extern "C" const PluginContext* falconpm_get_context(void);

// Expose Peregrine and Merlin APIs
extern "C" PeregrineAPI peregrine_api;
extern "C" MerlinAPI merlin_api;

// Bootstrap wrappers (from rAthena side)
extern "C" {
    int fpm_get_sd_x(struct map_session_data* sd);
    int fpm_get_sd_y(struct map_session_data* sd);
    int fpm_get_sd_m(struct map_session_data* sd);

    int fpm_get_bl_x(struct block_list* bl);
    int fpm_get_bl_y(struct block_list* bl);
    int fpm_get_bl_id(struct block_list* bl);

    const char* fpm_get_map_name(int m);

    struct block_list* fpm_get_nearest_mob(struct map_session_data* sd, int range);
    int fpm_unit_attack(struct map_session_data* sd, struct block_list* target);

    struct block_list* fpm_get_nearest_item(struct map_session_data* sd, int range);
    int fpm_pc_loot_item(struct map_session_data* sd, struct block_list* item);

    void fpm_send_message(struct map_session_data* sd, const char* msg);
    int fpm_get_account_id(struct map_session_data* sd);
    struct map_session_data* fpm_map_id2sd(int account_id);

    int fpm_pc_walktoxy(struct map_session_data* sd, short x, short y, int type);
    int fpm_unit_walktoxy(struct block_list* bl, short x, short y, unsigned char flag);
    int fpm_path_search(struct walkpath_data* wpd, int m,
                        int x0, int y0, int x1, int y1, int flag);
    const int16_t* fpm_get_dirx();
    const int16_t* fpm_get_diry();

    int fpm_add_timer(uint64_t tick,
                      int (*func)(int, uint64_t, int, intptr_t),
                      int id,
                      intptr_t data);
    uint64_t fpm_gettick(void);

    bool fpm_atcommand_register(const char* name, AtCmdFunc func);
    bool fpm_atcommand_unregister(const char* name);

    void mln_api_init(void);
}
