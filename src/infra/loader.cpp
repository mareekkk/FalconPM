#include <dlfcn.h>
#include <cstdio>
#include <vector>
#include <string>
#include "../infra/plugin_api.h"

extern "C" {

static void* g_base = nullptr;
static std::vector<void*> g_others;

static void* open_plugin(const char* path, int flags) {
    void* h = dlopen(path, flags);
    if (!h) {
        std::fprintf(stderr, "[FalconPM] dlopen failed for %s: %s\n", path, dlerror());
    } else {
        std::fprintf(stderr, "[FalconPM] loaded %s\n", path);
    }
    return h;
}

static PluginDescriptor* get_desc(void* h) {
    if (!h) return nullptr;
    void* sym = dlsym(h, "PLUGIN");
    if (!sym) {
        std::fprintf(stderr, "[FalconPM] missing PLUGIN symbol: %s\n", dlerror());
        return nullptr;
    }
    return reinterpret_cast<PluginDescriptor*>(sym);
}

// exported by falconpm_base.so
using get_ctx_t = const PluginContext* (*)();

 int falconpm_loader_init(void) {
    // --------------------------------------------------------------------
    // Guard against multiple loader init calls
    // --------------------------------------------------------------------
    static bool already_init = false;
    if (already_init) {
        fprintf(stderr, "[FalconPM] loader skipped (already initialized).\n");
        return 1;
    }

    already_init = true;

    void* base_handle = dlopen("plugins/falconpm_base.so", RTLD_NOW);
     if (!base_handle) {
         fprintf(stderr, "[FalconPM] dlopen failed for plugins/falconpm_base.so: %s\n", dlerror());
         return 0;
    }

    // fetch runtime context for other plugins
    const PluginContext* ctx = nullptr;
    if (auto get_ctx = (get_ctx_t)dlsym(g_base, "falconpm_get_context")) {
        ctx = get_ctx();
    }

    // 2) Load autoroute with RTLD_LOCAL (doesn’t need to export further)
    if (void* h = open_plugin("plugins/autoroute.so", RTLD_NOW | RTLD_LOCAL)) {
        if (auto* desc = get_desc(h)) {
            if (!desc->init(ctx)) {
                std::fprintf(stderr, "[FalconPM] autoroute init failed\n");
            } else {
                g_others.push_back(h);
            }
        }
    }

    // 3) Load autoattack with RTLD_LOCAL (new)
    if (void* h = open_plugin("plugins/autoattack.so", RTLD_NOW | RTLD_LOCAL)) {
        if (auto* desc = get_desc(h)) {
            if (!desc->init(ctx)) {
                std::fprintf(stderr, "[FalconPM] autoattack init failed\n");
            } else {
                g_others.push_back(h);
            }
        }
    }

    return 1;
}

void falconpm_loader_final(void) {
    // shutdown in reverse order
    for (auto it = g_others.rbegin(); it != g_others.rend(); ++it) {
        if (auto* desc = get_desc(*it)) desc->shutdown();
        if (*it) dlclose(*it);
    }
    g_others.clear();

    if (g_base) {
        if (auto* base = get_desc(g_base)) base->shutdown();
        dlclose(g_base);
        g_base = nullptr;
    }
}

int plugin_init(void)  { return falconpm_loader_init(); }
void plugin_final(void){ falconpm_loader_final(); }

} // extern "C"
