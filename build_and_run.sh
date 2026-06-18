#!/bin/zsh
set -e

PRESET="debug"
if [[ "$1" == "release" ]]; then
    PRESET="release"
fi

TARGET_DIR="build_${PRESET}"

if [ ! -d "$TARGET_DIR" ]; then
    chmod +x init.sh
    echo "${TARGET_DIR} directory not found. Bootstrapping via init.sh ..."
    ./init.sh "$PRESET"
fi

cd "$TARGET_DIR"
ninja SimpleEngine 

echo "--------------------------------------------------"
echo "Launching SimpleEngine (${PRESET})..."
echo "--------------------------------------------------"
./SimpleEngine
