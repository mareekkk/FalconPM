// rAthena shim → calls into FalconPM loader.so
#include <dlfcn.h>
#include <stdio.h>

extern "C" void try_load_falconpm() {
    void* handle = dlopen("plugins/falconpm_loader.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "[FalconPM] Failed to open falconpm_loader.so: %s\n", dlerror());
        return;
    }

    // Look for the exported entrypoint
    auto init = (void(*)())dlsym(handle, "falconpm_loader_init");
    if (init) {
        fprintf(stdout, "[FalconPM] Loader entry found, initializing...\n");
        init();
    } else {
        fprintf(stderr, "[FalconPM] falconpm_loader_init not found in loader.so\n");
    }
}
