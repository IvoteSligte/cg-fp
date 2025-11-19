#!/usr/bin/env bash

echo "Building project. Requires cmake 3.14 and OpenGL 4.3 or greater."

SCRIPT_DIR="$(dirname $0)"

cd "$SCRIPT_DIR"

cmake -S . -B build
cd build
make
cd ..
cp build/main .
