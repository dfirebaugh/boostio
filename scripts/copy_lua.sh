#!/usr/bin/bash

set -e

CONFIG_SRC="./lua/config"
CONFIG_DEST="$HOME/.config/boostio"
PLUGINS_SRC="./lua/plugins"
PLUGINS_DEST="$HOME/.local/share/boostio"
COMMANDS_SRC="./lua/commands"
COMMANDS_DEST="$HOME/.local/share/boostio"

echo "Installing Boostio configuration..."

mkdir -p "$CONFIG_DEST"
echo "Created config directory: $CONFIG_DEST"

if [ -d "$CONFIG_SRC" ]; then
    cp "$CONFIG_SRC"/*.lua "$CONFIG_DEST/" 2>/dev/null || echo "No .lua files found in $CONFIG_SRC"
    echo "Copied config files: $CONFIG_SRC/*.lua -> $CONFIG_DEST/"
fi

mkdir -p "$PLUGINS_DEST"
cp -r "$PLUGINS_SRC" "$PLUGINS_DEST/"
echo "Copied plugins: $PLUGINS_SRC -> $PLUGINS_DEST/"

cp -r "$COMMANDS_SRC" "$COMMANDS_DEST/"
echo "Copied commands: $COMMANDS_SRC -> $COMMANDS_DEST/"

echo "done!"
