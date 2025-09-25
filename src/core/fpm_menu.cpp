// src/core/fpm_menu.cpp
// Provides simple NPC-style menus through FalconPM

#include "falconpm.hpp"
#include "../infra/plugin_api.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>

// Bootstrap clif functions (declared in falconpm_bootstrap.cpp)
extern "C" {
    void fpm_clif_scriptmes(map_session_data* sd, const char* mes);
    void fpm_clif_scriptnext(map_session_data* sd, int npcid);
}

// Registry of callbacks per account
static std::unordered_map<int, void(*)(int,int)> menu_callbacks;

// Open a menu for a given account ID
static void fpm_open_menu(int account_id,
                          const char* title,
                          const char* options[],
                          int option_count,
                          void (*callback)(int,int))
{
    map_session_data* sd = fpm_map_id2sd(account_id);
    if (!sd) return;

    // Show menu title
    fpm_clif_scriptmes(sd, title);

    // Show options
    for (int i = 0; i < option_count; i++) {
        fpm_clif_scriptmes(sd, options[i]);
    }

    // Wait for choice
    fpm_clif_scriptnext(sd, 0);

    // Register callback
    menu_callbacks[account_id] = callback;

    std::printf("[FalconPM] Menu opened for account %d with %d options\n",
                account_id, option_count);
}

// Called when player selects a menu option
void fpm_menu_choice(int account_id, int choice) {
    auto it = menu_callbacks.find(account_id);
    if (it != menu_callbacks.end()) {
        // Call the stored callback
        it->second(account_id, choice);
        menu_callbacks.erase(it);
    }
}

// Export API struct with proper linkage
extern "C" {
    MenuAPI menu_api = {
        fpm_open_menu
    };
}
