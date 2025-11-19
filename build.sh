#!/usr/bin/env bash

echo "Building project. Required pre-installed dependencies: cmake 3.14, GLEW, OpenGL 4.3 or greater."

SCRIPT_DIR="$(dirname $0)"

cd "$SCRIPT_DIR"

cmake -S src -B build
cd build
make
cd ..
cp build/main .
