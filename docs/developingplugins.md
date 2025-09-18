# 🧩 Developing Plugins for FalconPM

FalconPM is the **plugin infrastructure for rAthena**, designed to make it easy to extend servers with modular, hot-loadable plugins.  
This guide explains how you (or any developer) can build and run your own FalconPM plugins.

---

## 📂 Project Structure

When FalconPM is added as a submodule, the layout looks like this:

rathena/
├── src/
│ ├── map/ # rAthena map-server core
│ ├── ... # other rAthena sources
│ └── plugins/
│ └── falconpm/ # FalconPM (cloned here)
│ ├── CMakeLists.txt # FalconPM plugin build rules
│ ├── src/
│ │ ├── infra/ # loader, plugin_api.hpp
│ │ └── plugins/ # plugins live here
│ │ ├── hello/ # example plugin
│ │ └── ...
│ └── docs/
└── build/ # build output

---

## 🚀 Writing a New Plugin

1. **Create a folder for your plugin** under FalconPM:

rathena/plugins/falconpm/src/plugins/myplugin/

2. **Add your plugin source file**:
```cpp
// myplugin.cpp
#include "plugin_api.hpp"

extern "C" void plugin_init(PluginAPI* api) {
    g_plugin_api = api;
    g_plugin_api->log_info("[MyPlugin] Hello from my plugin!\n");
}

Update FalconPM’s CMakeLists.txt to build it:

add_library(myplugin MODULE src/plugins/myplugin/myplugin.cpp)
set_target_properties(myplugin PROPERTIES PREFIX "" SUFFIX ".so")

🏗️ Building

From your rathena/ root:

cd build
cmake ..
make -j$(nproc) myplugin


This will produce:

build/plugins/falconpm/src/plugins/myplugin/myplugin.so

📦 Installing

Copy the built .so into your runtime plugins directory:

mkdir -p ~/rathena/plugins
cp build/plugins/falconpm/src/plugins/myplugin/myplugin.so ~/rathena/plugins/


Run the server (map-server will auto-load any .so it finds in that folder):

./map-server


You should see your plugin initialize:

[FalconPM] Loaded plugin: myplugin.so
[MyPlugin] Hello from my plugin!

🔒 Notes for Developers

No need to edit rAthena core — just add your plugin inside FalconPM.

Stable API — your code should only depend on functions exposed in plugin_api.hpp.

Closed source possible — you may distribute only the compiled .so if you want to keep your plugin private.

Debugging — recompile your plugin only (faster) with:

make myplugin