// /home/marek/FalconPM/src/infra/loader.cpp
#include <dlfcn.h>
#include <cstdio>
#include <vector>
#include <string>
#include "../infra/plugin_api.h"

extern "C" {

static void* g_base = nullptr;
static std::vector<void*> g_others;

static void* open_plugin(const char* path) {
    void* h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h) std::fprintf(stderr, "[FalconPM] dlopen failed for %s: %s\n", path, dlerror());
    else    std::fprintf(stderr, "[FalconPM] loaded %s\n", path);
    return h;
}

static PluginDescriptor* get_desc(void* h) {
    void* sym = dlsym(h, "PLUGIN");
    if (!sym) {
        std::fprintf(stderr, "[FalconPM] missing PLUGIN symbol: %s\n", dlerror());
        return nullptr;
    }
    return reinterpret_cast<PluginDescriptor*>(sym);
}

// exported by falconpm_base.so (we add this export there)
using get_ctx_t = const PluginContext* (*)();

int falconpm_loader_init(void) {
    // 1) load base
    g_base = open_plugin("plugins/falconpm_base.so");
    if (!g_base) return 0;

    auto* base = get_desc(g_base);
    if (!base) return 0;

    // init base first (it logs “initialized”)
    if (!base->init(nullptr)) {
        std::fprintf(stderr, "[FalconPM] base init failed\n");
        return 0;
    }

    // fetch runtime context for other plugins
    const PluginContext* ctx = nullptr;
    if (auto get_ctx = (get_ctx_t)dlsym(g_base, "fpm_get_context"))
        ctx = get_ctx();

    // 2) load autoroute and init with ctx
    if (void* h = open_plugin("plugins/autoroute.so")) {
        if (auto* desc = get_desc(h)) {
            if (!desc->init(ctx)) {
                std::fprintf(stderr, "[FalconPM] autoroute init failed\n");
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
