# FalconPM – Open-Source Plugin Manager for rAthena

FalconPM is an **open-source automation framework and plugin manager** for [rAthena](https://rathena.org).  
It provides a **unified way to build, load, and manage plugins** inside your server.

FalconPM is an open-source automation framework for the rAthena emulator.

- ⚡ Modular plugin system (auto-pots, auto-combat, auto-support)
- 🎮 Unified NPC manager for players
- 🔧 Stable API for developers
- 📜 Free under GPLv3

## ✨ Features
- **Plugin Manager Core** (`falconpm.c`)
  - Registers plugins dynamically (`/plugins/` folder).
  - Provides a shared API for configuration, logging, and NPC integration.
- **In-game Manager NPC** (`falconpm_manager.txt`)
  - Configure automation without touching config files.
  - Enable/disable plugins per-player or globally.
- **Starter Plugins** (included free, open source):
  - **Auto-Potions** – Humanized HP/SP pot usage.
  - **Auto-Combat** – Basic attack routines with delays/jitter.
  - **Auto-Support** – Buffs, heals, and party support.
- **Humanized Behavior**
  - Random delays, skips, and natural timing to avoid “robotic” play.
- **Extensible**
  - Drop any new plugin into `/plugins/` and register via FalconPM.
  - Developers can target FalconPM’s API instead of raw rAthena hooks.

## Quickstart

See [INSTALL.md](docs/INSTALL.md).

## Documentation

- [Installation](docs/INSTALL.md)
- [Usage Guide](docs/USAGE.md)
- [Plugin Catalog](docs/PLUGINS.md)
- [Developer Architecture](docs/ARCHITECTURE.md)
- [Roadmap](docs/ROADMAP.md)

## Contributing

See [CONTRIBUTING.md](docs/CONTRIBUTING.md).

## 🚀 Getting Started
1. Clone this repository into your rAthena `plugins/` folder.
2. Compile using the rAthena plugin build system.
3. Add `falconpm_manager.txt` to your server’s NPC scripts.
4. Recompile and run your server.
5. Configure automation in-game via the `@falconpm` NPC.

## 🔧 Developer API (Preview)
FalconPM exposes helper functions for plugin developers:
```c
// Register your plugin
falconpm_register_plugin("AutoPot", &plugin_info);

// Read/write config values
int threshold = falconpm_get_config("AutoPot", "hp_threshold");
falconpm_set_config("AutoPot", "hp_threshold", 50);

// Log to FalconPM’s unified system
falconpm_log_event("AutoPot", "Player %d used potion", sd->bl.id);
```
📜 Roadmap
- 🔄 Profile switching (different settings per scenario).
- 🗺 Adaptive map behavior.
- 📦 Inventory & consumable management.
- 🎉 Event mode awareness.
- 📊 Extended logging & monitoring.

📝 License
FalconPM is released under the **GPL v3 license**.  
This ensures the project remains free and open, while preventing others from taking the code private and selling it without sharing improvements.

🤝 Contributing
Contributions are welcome!

- Fork, add your plugin, and open a PR.  
- Follow the coding style in `ARCHITECTURE.md`.  

---

FalconPM is built by the community, for the community.  
Let’s give rAthena the open plugin ecosystem it deserves!
