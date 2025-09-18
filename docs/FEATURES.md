# FalconPM Features

FalconPM is a modular automation framework for rAthena/Hercules.  
It provides humanized automation plugins configurable through an in-game NPC (`falconpm_manager.txt`).

---

## ✅ Core Features

### Auto-Pots
- Automatically uses healing potions when HP is below threshold.
- Configurable potion item ID and HP % threshold.
- Humanized behavior:
  - Random delay before pot use.
  - Chance to hesitate (skip).
- NPC Options:
  - Enable/Disable
  - Configure (threshold & item)
  - View Current Settings
  - Reset Settings

---

### Auto-Combat
- Automatically attacks nearby monsters.
- Skill rotation system (up to 5 slots).
- Supports skill types:
  - Target skill
  - Ground skill (AOE on mob location)
  - Self-buff (checks status before casting)
- Randomized skill rotation order for humanization.
- Fallback to normal attack if no skill is valid.
- Configurable mob filter:
  - Modes: All / Whitelist / Blacklist
  - Up to 5 mob IDs (by ID or name)
- NPC Options:
  - Enable/Disable
  - Configure Skills (ID, level, type)
  - Configure Mob Filter (mode, mob list)
  - View Current Settings
  - Reset Settings

---

### Auto-Support
- Designed for slave priests and support classes.
- Casts buffs on self if not already active.
- Casts healing/support skills on party members under HP threshold.
- Humanized delays and skip chance.
- NPC Options:
  - Enable/Disable
  - View Current Settings
  - Reset Settings

---

### Auto-Loot
- Automatically picks up nearby dropped items (range = 3 cells).
- Humanized behavior:
  - Random delay before pickup.
  - Chance to skip (5%).
- Configurable weight % limit (default 90%).
- Configurable item filter:
  - Modes: All / Whitelist / Blacklist
  - Up to 5 item slots
  - Each slot supports **Max Count** (e.g. only pick up to 200 Red Potions).
- NPC Options:
  - Enable/Disable
  - Configure Weight Limit
  - Configure Item Filter (mode, slots by ID or name)
  - View Current Settings (shows item names, IDs, and max counts)
  - Reset Settings

---

### Logging
- Tracks player stats while automation is active.
- Records:
  - Kill count
  - Potions used
  - Total damage dealt
  - Playtime (since first activation)
- Command: `@autolog` to view stats in-game.

---

## 🧩 Design Notes
- Each plugin is modular (`autocombat.c`, `autopots.c`, `autosupport.c`, `autoloot.c`, `logging.c`).
- Humanized behavior everywhere (delays, skips, jitter).
- NPC Manager provides unified configuration.
- All settings stored in account variables (`#auto_*`).

---

## 📌 Roadmap (Next Features)
- Profile switching (multiple saved configs).
- Adaptive behavior per map.
- Advanced auto-storage (restocking, selling, dropping).
- Event awareness (change behavior during events).
- Zeny/value-based loot filtering.
- Dashboard-style NPC landing page.

