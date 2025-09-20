#include "plugin_api.h"
#include <dlfcn.h>
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <vector>

// ----------------------
// Logging
// ----------------------
static void log_info(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}
static void log_error(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// ----------------------
// Error codes
// ----------------------
typedef enum {
    FPM_OK = 0,
    FPM_ERR_INCOMPATIBLE_VERSION,
    FPM_ERR_MISSING_MODULE,
    FPM_ERR_SYMBOL_RESOLVE,
    FPM_ERR_PLUGIN_INIT,
    FPM_ERR_DLOPEN_FAILED,
    FPM_ERR_DESCRIPTOR_MISSING
} FpmError;

static const char* fpm_errstr(FpmError e) {
    switch (e) {
        case FPM_OK: return "OK";
        case FPM_ERR_INCOMPATIBLE_VERSION: return "Incompatible ABI version";
        case FPM_ERR_MISSING_MODULE: return "Missing required module";
        case FPM_ERR_SYMBOL_RESOLVE: return "Failed to resolve symbol";
        case FPM_ERR_PLUGIN_INIT: return "Plugin init() failed";
        case FPM_ERR_DLOPEN_FAILED: return "dlopen() failed";
        case FPM_ERR_DESCRIPTOR_MISSING: return "PluginDescriptor missing";
        default: return "Unknown error";
    }
}

// ----------------------
// Loaded plugin struct
// ----------------------
struct LoadedPlugin {
    void* handle;
    const PluginDescriptor* desc;
};

// Active plugins list
static std::vector<LoadedPlugin> plugins;

// ----------------------
// Load single plugin
// ----------------------
static FpmError load_plugin(const char* path) {
    log_info("[FalconPM] Loading plugin: %s", path);

    void* handle = dlopen(path, RTLD_NOW);
    if (!handle) {
        log_error("[FalconPM] dlopen failed: %s", dlerror());
        return FPM_ERR_DLOPEN_FAILED;
    }

    auto* desc = (const PluginDescriptor*)dlsym(handle, "fpm_plugin_descriptor");
    if (!desc) {
        log_error("[FalconPM] Missing PluginDescriptor in %s", path);
        dlclose(handle);
        return FPM_ERR_DESCRIPTOR_MISSING;
    }

    // Check required modules
    if (desc->required_modules) {
        size_t count = 0;
        const FpmModuleId* reqs = desc->required_modules(&count);
        for (size_t i = 0; i < count; i++) {
            if (reqs[i] >= FPM_MOD__COUNT) {
                log_error("[FalconPM] %s requires unknown module %d", desc->name, (int)reqs[i]);
                dlclose(handle);
                return FPM_ERR_MISSING_MODULE;
            }
        }
    }

    // Build PluginContext
    PluginContext ctx{};
    ctx.api = {FALCONPM_API_VERSION_MAJOR, FALCONPM_API_VERSION_MINOR};
    // TODO: allocate + populate module tables here

    // Call init()
    if (!desc->init(&ctx)) {
        log_error("[FalconPM] Plugin %s init() failed", desc->name);
        dlclose(handle);
        return FPM_ERR_PLUGIN_INIT;
    }

    plugins.push_back({handle, desc});
    log_info("[FalconPM] Plugin %s loaded successfully", desc->name);
    return FPM_OK;
}

// ----------------------
// Unload all plugins
// ----------------------
static void unload_plugins() {
    for (auto& p : plugins) {
        if (p.desc && p.desc->shutdown) {
            p.desc->shutdown();
        }
        dlclose(p.handle);
    }
    plugins.clear();
}

// ----------------------
// Load all from ./plugins
// ----------------------
void load_all_plugins() {
    DIR* dir = opendir("./plugins");
    if (!dir) {
        log_error("[FalconPM] Could not open plugins/ directory");
        return;
    }

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (strstr(ent->d_name, ".so")) {
            std::string path = std::string("./plugins/") + ent->d_name;
            FpmError err = load_plugin(path.c_str());
            if (err != FPM_OK) {
                log_error("[FalconPM] Failed to load %s: %s", ent->d_name, fpm_errstr(err));
            }
        }
    }
    closedir(dir);
}
