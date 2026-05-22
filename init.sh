#!/bin/zsh

if [ -d "build" ]; then
    echo "Found existing build folder. Deleting..."
    rm -rf build
fi

cmake --preset=default
echo "Project initialized, build folder created"
