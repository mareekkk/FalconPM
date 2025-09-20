#include "../infra/plugin_api.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <unordered_map>
#include <string>
#include <dlfcn.h>

// Forward from rAthena
#include "atcommand.hpp"

// ----------------------------------------------------
// Logging
// ----------------------------------------------------
static void log_info_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

static void log_error_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// ----------------------------------------------------
// Dummy stubs (will be replaced by rAthena hooks later)
// ----------------------------------------------------
static struct block_list* dummy_get_target(void* u) {
    (void)u;
    return nullptr;
}

static int dummy_get_id(struct block_list* bl) {
    (void)bl;
    return 0;
}

static int dummy_get_type(struct block_list* bl) {
    (void)bl;
    return 0;
}

static struct map_session_data* dummy_map_id2sd(int aid) {
    (void)aid;
    return nullptr;
}

static void dummy_send_message(int fd, const char* msg) {
    (void)fd;
    printf("[MSG] %s\n", msg);
}

static int32_t rnd_impl(void) {
    return rand();
}

// ----------------------------------------------------
// FalconPM Atcommand registry
// ----------------------------------------------------
static std::unordered_map<std::string, AtCmdFunc> fpm_atcmds;

// Pointer to original rAthena dispatcher
using is_atcommand_t = bool(*)(const int32, map_session_data*, const char*, int32);
static is_atcommand_t orig_is_atcommand = nullptr;

// Hooked dispatcher
bool is_atcommand(const int32 fd, map_session_data* sd, const char* message, int32 type) {
    // Strip leading symbol (@/#)
    if (message && (message[0] == '@' || message[0] == '#')) {
        std::string cmd = message + 1; // skip symbol
        auto it = fpm_atcmds.find(cmd);
        if (it != fpm_atcmds.end()) {
            fprintf(stdout, "[falconpm_base] plugin atcommand: %s\n", cmd.c_str());
            it->second(sd, cmd.c_str(), ""); // call plugin handler
            return true;
        }
    }

    // Fallback to original dispatcher if available
    if (!orig_is_atcommand) {
        orig_is_atcommand = (is_atcommand_t)dlsym(RTLD_NEXT, "is_atcommand");
    }
    if (orig_is_atcommand) {
        return orig_is_atcommand(fd, sd, message, type);
    }

    return false; // no handler found
}


// ----------------------------------------------------
// Atcommand wrappers for FalconPM plugins
// ----------------------------------------------------
extern "C" void atcommand_register(const char* name, AtCommandFunc func);
extern "C" bool atcommand_unregister(const char* name); // optional, if you patch one

static bool at_add_wrapper(const char* name, AtCmdFunc func) {
    if (!name || !func) return false;
    atcommand_register(name, func);
    fprintf(stdout, "[falconpm_base] registered atcommand: %s\n", name);
    return true;
}

static bool at_remove_wrapper(const char* name) {
    if (!name) return false;
    if (atcommand_unregister && atcommand_unregister(name)) {
        fprintf(stdout, "[falconpm_base] removed atcommand: %s\n", name);
        return true;
    }
    return false;
}


// ----------------------------------------------------
// API tables
// ----------------------------------------------------
static LogAPI log_api = {
    { sizeof(LogAPI), {1,0} },
    log_info_impl,
    log_error_impl
};

static UnitAPI unit_api = {
    { sizeof(UnitAPI), {1,0} },
    dummy_get_target,
    dummy_get_id,
    dummy_get_type
};

static PlayerAPI player_api = {
    { sizeof(PlayerAPI), {1,0} },
    dummy_map_id2sd,
    dummy_send_message
};

static RandomAPI rnd_api = {
    { sizeof(RandomAPI), {1,0} },
    rnd_impl
};

static AtcommandAPI atcommand_api = {
    { sizeof(AtcommandAPI), {1,0} },
    at_add_wrapper,
    at_remove_wrapper
};

// ----------------------------------------------------
// Context given to plugins
// ----------------------------------------------------
static PluginContext g_ctx = {
    {1,0}, // ABI version
    &log_api,
    &unit_api,
    &player_api,
    &rnd_api,
    &atcommand_api
};

// ----------------------------------------------------
// Base plugin descriptor
// ----------------------------------------------------
static const FpmModuleId* required_modules(size_t* count) {
    *count = 0;
    return nullptr;
}

static bool init(const PluginContext* ctx) {
    (void)ctx;
    log_api.info("[falconpm_base] initialized");
    return true;
}

static void shutdown(void) {
    log_api.info("[falconpm_base] shutdown");
    fpm_atcmds.clear();
}

extern "C" {
PluginDescriptor PLUGIN = {
    "falconpm",
    "0.1",
    required_modules,
    init,
    shutdown
};
}
