# Computer Graphics Final Project: Voxel Raytraced Radiosity

This repository contains a simple shader-based software renderer that lights the scene using radiosity, with light distribution between patches (faces of voxels) performed through raycasting.

## Required Dependencies

To run this project, cmake 3.14+ and OpenGL 4.3+ need to be installed.
OpenGL must have the GL_ARB_storage_buffer and GL_ARB_direct_storage_access extensions, which most hardware does.

This project has only been tested on Linux, and the cmake build settings are steered towards it, but there is a small chance it will work on Windows and MacOS as well.

## Usage

There are `build.sh` and `run.sh` scripts provided for Linux.

Alternatively, one can manually run `cmake -S src -B build`, which creates an executable at `build/main`.

The cmake build script takes a while to run as it builds all dependencies, most notably SDL2, from source.

## Controls

Move: WASD + mouse
Switch to next scene: N

