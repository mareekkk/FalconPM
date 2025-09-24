#pragma once

#include <unordered_set>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize FalconPM config system: ensure directories and default files exist.
void skr_conf_init(void);

// Load per-account config from YAML (call on login).
void fpm_conf_load(int account_id);

// Save per-account config to YAML (enabled = false; configure only).
void fpm_conf_save(int account_id);

// Save + enable autoattack in YAML (enabled = true; start immediately).
void fpm_conf_start(int account_id);

// Query: is autoattack enabled for this account (current session)?
bool fpm_autoattack_enabled(void);

// Query: is this mob id allowed by the current filter?
// If filter is empty => return true (attack all).
bool fpm_mob_allowed(int mob_id);

// ADDED: Helper functions for UI interaction
void fpm_conf_set_filter(const std::unordered_set<int>& ids);
const std::unordered_set<int>& fpm_conf_get_filter(void);

#ifdef __cplusplus
}
#endif