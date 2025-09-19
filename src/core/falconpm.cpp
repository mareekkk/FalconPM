/**
 * FalconPM Base: plugin registry only (no global tick loop)
 */
#include "common/showmsg.hpp"
#include <cstdlib>
#include <ctime>

#define MAX_FALCONPM_PLUGINS 32
typedef void (*falconpm_callback)(void);

static falconpm_callback falconpm_plugins[MAX_FALCONPM_PLUGINS];
static int falconpm_count = 0;

// Exported so plugins can register themselves
extern "C" __attribute__((visibility("default")))
void falconpm_register(const char *name, falconpm_callback cb) {
    if (falconpm_count < MAX_FALCONPM_PLUGINS) {
        falconpm_plugins[falconpm_count++] = cb;
        ShowInfo("[FalconPM] Registered plugin: %s\n", name);
    }
}

// Initialization
static void falconpm_init(void) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    ShowInfo("[FalconPM] Base initialized\n");
}

// Called by loader
extern "C" __attribute__((visibility("default")))
void plugin_init(void) {
    falconpm_init();
}
