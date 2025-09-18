#!/bin/bash
set -e

# Get absolute path to rAthena root (one level up from falconpm/)
RATHENA_DIR="$(cd "$(dirname "$0")/.." && pwd)"

echo "==> FalconPM setup started"
echo "rAthena root: $RATHENA_DIR"

# Create plugin folder inside rAthena
mkdir -p "$RATHENA_DIR/src/plugins/falconpm"

# Copy FalconPM core and plugin sources
cp -u "$RATHENA_DIR/falconpm/base/"*.c "$RATHENA_DIR/src/plugins/falconpm/"
cp -u "$RATHENA_DIR/falconpm/plugins/"*.c "$RATHENA_DIR/src/plugins/falconpm/"

# Copy NPC
mkdir -p "$RATHENA_DIR/npc/custom"
cp -u "$RATHENA_DIR/falconpm/npc/falconpm_manager.txt" "$RATHENA_DIR/npc/custom/"

# Copy DB
mkdir -p "$RATHENA_DIR/db/import"
cp -u "$RATHENA_DIR/falconpm/db/fpm_portals.txt" "$RATHENA_DIR/db/import/"

# Write plugin + NPC config
tee "$RATHENA_DIR/conf/import/falconpm_plugins.conf" > /dev/null <<'EOF'
// FalconPM Core
plugin: falconpm/falconpm
plugin: falconpm/fpm_graph
plugin: falconpm/fpm_path
plugin: falconpm/fpm_portals
plugin: falconpm/fpm_route

// FalconPM Modules
plugin: falconpm/autocombat
plugin: falconpm/autoloot
plugin: falconpm/autoroute
plugin: falconpm/autostorage
plugin: falconpm/logging
plugin: falconpm/partysupport
plugin: falconpm/status

// NPC
npc: npc/custom/falconpm_manager.txt
EOF

echo "==> FalconPM setup complete!"
echo "Next: cd $RATHENA_DIR && make clean && make server && ./athena-start restart"
