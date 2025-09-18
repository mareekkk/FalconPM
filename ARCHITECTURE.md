# 🏗 FalconPM Architecture

FalconPM is a modular automation framework for **rAthena** servers.  
It is designed as a **starter kit** for server owners who want safe, extensible player automation with human-like behavior.

---

## 🔌 Core Design

FalconPM runs as a **base C plugin** (`falconpm.c`) that:
- Registers itself with rAthena’s plugin system.
- Exposes helper functions for automation plugins.
- Maintains a **unified configuration and logging system**.

### Plugin Lifecycle
1. **Registration**  
   ```c
   falconpm_register_plugin("AutoPot", &plugin_info);
   ```
2. **Configuration**  
   ```c
   int threshold = falconpm_get_config("AutoPot", "hp_threshold");
   falconpm_set_config("AutoPot", "hp_threshold", 50);
   ```
3. **Execution**  
   Plugin hooks into rAthena events (`battle`, `status`, `unit`) and runs automation logic with built-in delays, skips, and jitter to mimic real players.

---

## 📦 Plugins

Plugins live in `plugins/` and can be dropped in or removed freely.

Current modules:
- **Auto-Pot** → Uses consumables when HP/SP drop below threshold.  
- **Auto-Combat** → Attacks nearby monsters, obeying map & targeting rules.  
- **Auto-Support** → Casts buffs/heals intelligently on party members.  

Each plugin:
- Has its own config section (managed via NPC).
- Uses FalconPM’s logging system for consistent reporting.
- Can be extended with minimal boilerplate.

---

## 🧑‍🤝‍🧑 NPC Interface

Players configure FalconPM via the **FalconPM Manager NPC** (`npc/custom/falconpm_manager.txt`).

Features:
- Enable/disable plugins in-game.  
- Adjust thresholds, target lists, and support options.  
- Save/load settings per character.  

This avoids reliance on external config files and gives players control through familiar game interactions.

---

## 🛠 Humanized Behavior

Unlike mechanical bots, FalconPM is designed to **feel like a real player**:
- Adds random delays between actions.  
- Skips actions occasionally (not 100% efficiency).  
- Varies targeting and support order.  
- Honors rAthena’s rules and mechanics.

This makes automation safer and more natural on live servers.

---

## 🔧 Developer API (Preview)

Helper functions for plugin developers:

```c
// Register your plugin
falconpm_register_plugin("AutoPot", &plugin_info);

// Read/write config values
int threshold = falconpm_get_config("AutoPot", "hp_threshold");
falconpm_set_config("AutoPot", "hp_threshold", 50);

// Log to FalconPM’s unified system
falconpm_log_event("AutoPot", "Player %d used potion", sd->bl.id);
```

---

## 🗺 Roadmap

- 🔄 Profile switching (different settings per scenario).  
- 🗺 Adaptive map behavior.  
- 📦 Inventory & consumable management.  
- 🎉 Event mode awareness.  
- 📊 Extended logging & monitoring.  

---
