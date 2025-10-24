#!/usr/bin/env bash

SCRIPT_DIR="$(dirname $0)"

cd "$SCRIPT_DIR"

cmake -S src -B build
cd build
make
cd ..
cp build/main .
