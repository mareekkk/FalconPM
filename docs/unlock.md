# FalconPM Unlock System

FalconPM includes a **flexible unlock system** so server owners can decide:
- Who can use automation,
- Which plugins are enabled,
- Whether access is permanent or time-limited,
- Whether access comes from one VIP item or split bundles.

---

## 📑 Table of Contents
1. [Core Concepts](#-core-concepts)
2. [Plugin Runtime Check](#-plugin-runtime-check)
3. [Default Unlock Items](#-default-unlock-items)
   - [FalconPM VIP License (all features)](#1-falconpm-vip-license-all-features)
   - [Falcon Combat+Loot Bundle](#2-falcon-combatloot-bundle)
   - [Falcon Support License](#3-falcon-support-license-party-buffsheal-only)
   - [Falcon VIP 30-Day License](#4-falcon-vip-30-day-license-all-features-time-limited)
   - [Falcon Combat+Loot+Storage Bundle](#5-falcon-combatlootstorage-bundle)
4. [Other Unlock Methods](#-other-unlock-methods)
5. [Why This System Works](#-why-this-system-works)

---

## 🔑 Core Concepts

FalconPM plugins check **per-player unlock flags**:

- `#falconpm_unlocked` → **master flag** (1 = FalconPM allowed).
- `#falconpm_license_expiry` → **optional expiry timestamp** (Unix time).
- Per-plugin unlocks:
  - `#falconpm_unlock_pots` → Auto-Pots
  - `#falconpm_unlock_combat` → Auto-Combat
  - `#falconpm_unlock_support` → Auto-Support
  - `#falconpm_unlock_loot` → Auto-Loot
  - `#falconpm_unlock_storage` → Auto-Storage

---

## ⚙ Plugin Runtime Check

Every FalconPM plugin begins with:

```c
// Master unlock
if (!pc_readaccountreg(sd, script->add_str("#falconpm_unlocked")))
    return;

// Expiry check
int expiry = pc_readaccountreg(sd, script->add_str("#falconpm_license_expiry"));
if (expiry > 0 && (int)time(NULL) > expiry)
    return;

// Per-plugin check (example: Auto-Combat)
if (!pc_readaccountreg(sd, script->add_str("#falconpm_unlock_combat")))
    return;

## 📦 Default Unlock Items

### 1. FalconPM VIP License (all features)
6001,FalconVIP,Falcon VIP License,11,100,,10,,,,,0xFFFFFFFF,7,2,,,,,,
{ set #falconpm_unlocked,1;
  set #falconpm_unlock_pots,1;
  set #falconpm_unlock_combat,1;
  set #falconpm_unlock_support,1;
  set #falconpm_unlock_loot,1;
  set #falconpm_unlock_storage,1; },{}

---

### 2. Falcon Combat+Loot Bundle
6002,FalconCL,Falcon Combat+Loot,11,100,,10,,,,,0xFFFFFFFF,7,2,,,,,,
{ set #falconpm_unlocked,1;
  set #falconpm_unlock_combat,1;
  set #falconpm_unlock_loot,1; },{}

---

### 3. Falcon Support License (party buffs/heal only)
6003,FalconSUP,Falcon Support License,11,100,,10,,,,,0xFFFFFFFF,7,2,,,,,,
{ set #falconpm_unlocked,1;
  set #falconpm_unlock_support,1; },{}

---

### 4. Falcon VIP 30-Day License (all features, time-limited)
6004,FalconVIP30,Falcon VIP 30 Days,11,100,,10,,,,,0xFFFFFFFF,7,2,,,,,,
{ set #falconpm_unlocked,1;
  set #falconpm_license_expiry,gettimetick(2)+2592000; // 30 days
  set #falconpm_unlock_pots,1;
  set #falconpm_unlock_combat,1;
  set #falconpm_unlock_support,1;
  set #falconpm_unlock_loot,1;
  set #falconpm_unlock_storage,1; },{}

---

### 5. Falcon Combat+Loot+Storage Bundle
6005,FalconCLS,Falcon Combat+Loot+Storage,11,100,,10,,,,,0xFFFFFFFF,7,2,,,,,,
{ set #falconpm_unlocked,1;
  set #falconpm_unlock_combat,1;
  set #falconpm_unlock_loot,1;
  set #falconpm_unlock_storage,1; },{}

---

## 🛠 Other Unlock Methods
- **Item unlock** → integrate with existing shop items.
- **GM Commands** (planned):
  @falconpm grant <player> <days>
  @falconpm revoke <player>
  @falconpm status <player>
- **Admin NPC** → GM-only menu to grant/revoke/extend licenses.

---

## ✅ Why This System Works
- **Drop-in replacement** → servers with existing automation items just change their item script to set FalconPM flags.
- **Flexible** → admins can make bundles (VIP = all, or combat+loot only).
- **Supports subscriptions** → expiry system built-in.
- **Admin-friendly** → can be controlled via item, NPC, or GM commands.
- **Secure** → plugins always check flags server-side, no bypass.
