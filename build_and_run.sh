#!/bin/zsh

if [ ! -d "build" ]; then
    chmod +x init.sh
    echo "Build directory not found. Running init.sh ..."
    ./init.sh
fi

cd build
ninja && ./SimpleEngine
