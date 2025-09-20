#include "../infra/plugin_api.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

// ----------------------------------------------------
// Concrete implementations (adapt to real rAthena later)
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

// ----------------------------------------------------
// Context given to plugins
// ----------------------------------------------------
static PluginContext g_ctx = {
    {1,0}, // ABI version
    &log_api,
    &unit_api,
    &player_api,
    &rnd_api
};

// ----------------------------------------------------
// Required module list for base (none)
// ----------------------------------------------------
static const FpmModuleId* required_modules(size_t* count) {
    *count = 0;
    return nullptr;
}

// ----------------------------------------------------
// Init / Shutdown
// ----------------------------------------------------
static bool init(const PluginContext* ctx) {
    (void)ctx; // base has nothing extra
    log_api.info("[falconpm] initialized");
    return true;
}

static void shutdown(void) {
    log_api.info("[falconpm] shutdown");
}

// ----------------------------------------------------
// Exported descriptor (⚠️ no extra extern keyword!)
// ----------------------------------------------------
extern "C" {
PluginDescriptor PLUGIN = {
    "falconpm",
    "0.1",
    required_modules,
    init,
    shutdown
};
}
