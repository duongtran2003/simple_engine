#!/bin/zsh
set -e

PRESET="debug"
if [[ "$1" == "release" ]]; then
    PRESET="release"
fi

TARGET_DIR="build_${PRESET}"

if [ -d "$TARGET_DIR" ]; then
    echo "Found existing ${TARGET_DIR} folder. Deleting..."
    rm -rf "$TARGET_DIR"
fi

echo "Initializing project with preset: ${PRESET} ..."
cmake --preset=$PRESET

echo "Project initialized! Folder ./${TARGET_DIR} created successfully."
