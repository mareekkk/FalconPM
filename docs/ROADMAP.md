# FalconPM Roadmap

FalconPM aims to provide a modular, humanized automation system for rAthena/Hercules servers.  
This roadmap includes current features and planned enhancements, inspired by OpenKore’s flexibility.

---

## ✅ Phase 1 – Core Features (Implemented)

- **Auto-Pots**
  - Threshold-based potion use
  - Humanization (delays, skip chance)

- **Auto-Combat**
  - Skill rotation (5 slots, target/ground/self-buff types)
  - Fallback to normal attack
  - Mob filtering (all / whitelist / blacklist, up to 5 slots)
  - Humanized action order (randomized, delays)

- **Auto-Support**
  - Self-buffs if not active
  - Heal/buff party members under HP threshold
  - Humanization (delays, skip chance)

- **Auto-Loot**
  - Pickup items within normal range (3 cells)
  - Humanization (delays, skip chance)
  - Weight % limit (default 90)
  - Whitelist / Blacklist (5 slots)
  - Max count per item slot

- **Logging**
  - Tracks kills, potions used, damage, playtime
  - `@autolog` command to view stats

---

## 🚀 Phase 2 – Immediate Extensions

- **NPC Manager Enhancements**
  - Dashboard status page
  - View/Reset options for all modules
  - Mob & item search by name (already in design)

- **Profile Switching**
  - Multiple saved profiles (e.g. “Aggressive”, “Conservative”)
  - Switch via NPC or command (`@autoprofile`)

- **View Settings Improvements**
  - Show skill, mob, and item **names** instead of IDs
  - Clearer display of filter mode and limits

---

## 🔮 Phase 3 – Advanced Features (Inspired by OpenKore)

### Combat
- Conditional skill usage:
  - Use Heal if HP < 50%
  - Use SP recovery skill if SP < 30%
- Prioritize skills based on cooldowns/SP cost
- Adaptive behavior per map (different configs in dungeons vs fields)
- Auto-flee when HP/SP critical
- Detect nearby GMs/players → pause or behave human-like
- Event-awareness (special handling for event mobs, bosses)

### Loot
- Item value filter:
  - Only pick items above X zeny (from item_db)
- Item priority system:
  - Loot rare items before common drops
- Weight-aware pickup:
  - Drop lowest-priority items if overweight
- Inventory count limits (per item, already partially implemented)
- Integration with storage/sell:
  - Auto-store at Kafra when overweight
  - Auto-sell junk items

### Support
- Smarter buff scheduling:
  - Recast buffs only when close to expiration
- Conditional support:
  - Heal party members only if safe
- Auto-resurrect party members if class can

### General / Humanization
- Random idle times
- Occasional emotes or chat messages
- Variable walking patterns instead of straight lines
- Action latency simulation (skill → delay → cast)

### Logging / Analytics
- Per-session summary report
- Export logs to file
- Track loot/zeny gained
- Track EXP per hour

---

## 📌 Long-Term Goals
- Config import/export (share settings between characters)
- Web dashboard (remote monitoring)
- Integration with server events (custom hooks)
- Extensible plugin slots (drop-in new modules)
- Developer API for scripting conditions (mini rule engine)

---

## ⚠️ Notes
- FalconPM is not intended as a raw bot.  
- It provides **automation with humanization**, configurable in-game, meant for servers where such features are desired/allowed.  
- Many advanced features are inspired by OpenKore but will be re-implemented in a server-side, humanized way.

