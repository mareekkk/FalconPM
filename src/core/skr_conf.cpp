#include "skr_conf.h"
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <cstdio>

// If your build links yaml-cpp (recommended)
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

// ------------------------------------------------------------------
// Paths
// ------------------------------------------------------------------
static const char* BASE_DIR = "falconpm";
static const char* CONF_DIR = "falconpm/conf";
static const char* NPC_DIR  = "falconpm/npc";
static const char* AA_MENU  = "falconpm/npc/aa_conf.txt";

// ------------------------------------------------------------------
// In-memory state (session-local)
// ------------------------------------------------------------------
static std::unordered_set<int> s_mob_filter_ids;
static bool s_autoattack_enabled = false;

// We fetch account id from the same global used by Merlin.
extern int g_autoattack_account_id;

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------
static void ensure_dir(const char* path) {
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
            std::printf("[FalconPM] Created directory: %s\n", path);
        }
    } catch (const fs::filesystem_error& e) {
        std::printf("[FalconPM] Failed to create directory %s: %s\n", path, e.what());
    }
}

static void ensure_file(const char* path, const std::vector<std::string>& default_lines) {
    try {
        if (!fs::exists(path)) {
            std::ofstream fout(path);
            for (const auto& l : default_lines) fout << l << "\n";
            fout.close();
            std::printf("[FalconPM] Created default file: %s\n", path);
        }
    } catch (const std::exception& e) {
        std::printf("[FalconPM] Failed to create file %s: %s\n", path, e.what());
    }
}

static std::string conf_path_for(int account_id) {
    std::ostringstream oss;
    oss << CONF_DIR << "/" << account_id << ".yml";
    return oss.str();
}

// ------------------------------------------------------------------
// API
// ------------------------------------------------------------------
void skr_conf_init(void) {
    ensure_dir(BASE_DIR);
    ensure_dir(CONF_DIR);
    ensure_dir(NPC_DIR);

    // Provide a default text menu (admins can translate/edit later).
    ensure_file(AA_MENU, {
        "=== AutoAttack Config ===",
        "[ Filter Monster ]",
        "[ Show Monster Filtered ]",
        "[ Save ]",
        "[ Start ]",
        "[ Cancel ]"
    });

    std::printf("[FalconPM] Config system initialized.\n");
}

void fpm_conf_load(int account_id) {
    s_mob_filter_ids.clear();
    s_autoattack_enabled = false;

    if (account_id < 0) return;
    const std::string path = conf_path_for(account_id);

    std::ifstream fin(path);
    if (!fin.good()) {
        // Create a default disabled file to be explicit.
        YAML::Node root;
        YAML::Node aa;
        aa["enabled"] = false;
        aa["mob_filter"] = YAML::Node(YAML::NodeType::Sequence);
        root["autoattack"] = aa;

        std::ofstream fout(path);
        fout << root;
        fout.close();

        std::printf("[FalconPM] Created default config for account %d\n", account_id);
        return;
    }

    try {
        YAML::Node root = YAML::Load(fin);
        fin.close();

        if (root["autoattack"]) {
            auto node = root["autoattack"];
            s_autoattack_enabled = node["enabled"].as<bool>(false);

            if (node["mob_filter"]) {
                for (auto m : node["mob_filter"]) {
                    s_mob_filter_ids.insert(m.as<int>());
                }
            }
        }

        std::printf("[FalconPM] Loaded config %s (enabled=%d, %zu mobs)\n",
                    path.c_str(), s_autoattack_enabled ? 1 : 0, s_mob_filter_ids.size());
    } catch (const std::exception& e) {
        std::printf("[FalconPM] Failed to parse %s: %s\n", path.c_str(), e.what());
        // fallback to safe defaults
        s_mob_filter_ids.clear();
        s_autoattack_enabled = false;
    }
}

void fpm_conf_save(int account_id) {
    if (account_id < 0) return;

    YAML::Node root;
    YAML::Node aa;
    aa["enabled"] = false; // Save only
    YAML::Node list(YAML::NodeType::Sequence);
    for (int id : s_mob_filter_ids) list.push_back(id);
    aa["mob_filter"] = list;
    root["autoattack"] = aa;

    const std::string path = conf_path_for(account_id);
    std::ofstream fout(path);
    fout << root;
    fout.close();

    s_autoattack_enabled = false;
    std::printf("[FalconPM] Config saved (disabled) for account %d (%zu mobs)\n",
                account_id, s_mob_filter_ids.size());
}

void fpm_conf_start(int account_id) {
    if (account_id < 0) return;

    YAML::Node root;
    YAML::Node aa;
    aa["enabled"] = true; // Save + enable
    YAML::Node list(YAML::NodeType::Sequence);
    for (int id : s_mob_filter_ids) list.push_back(id);
    aa["mob_filter"] = list;
    root["autoattack"] = aa;

    const std::string path = conf_path_for(account_id);
    std::ofstream fout(path);
    fout << root;
    fout.close();

    s_autoattack_enabled = true;
    std::printf("[FalconPM] Config saved and started for account %d (%zu mobs)\n",
                account_id, s_mob_filter_ids.size());
}

bool fpm_autoattack_enabled(void) {
    return s_autoattack_enabled;
}

bool fpm_mob_allowed(int mob_id) {
    // Empty filter -> attack all (your requirement)
    if (s_mob_filter_ids.empty()) return true;
    return s_mob_filter_ids.count(mob_id) > 0;
}
