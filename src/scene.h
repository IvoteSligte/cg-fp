#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <fstream>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

const GLfloat QUAD_VERTICES[] = {
    -1.0, -1.0,
    1.0, -1.0,
    -1.0, 1.0,
    -1.0, 1.0,
    1.0, 1.0,
    1.0, -1.0
};

// mirrored with shaders
// alignas(16) is necessary to prevent layout optimizations
struct alignas(16) Voxel {
    alignas(16) glm::vec3 emission;
    alignas(16) glm::vec3 diffuse;
    // Double-buffered color.
    // The read index is indicated by dbColorReadIdx.
    alignas(16) glm::vec3 dbColor[2];
    // bit 0 set indicates that the voxel exists
    alignas(16) glm::uint flags;
};

// mirrored with shaders
// size of the voxel chunk in one dimension
const long CHUNK_SIZE = 32;

// mirrored with shaders
const int STORAGE_BUFFER_BINDING = 0;
const glm::uvec3 WORKGROUP_SIZE = glm::uvec3(8, 8, 8);

inline float randf()
{
    auto gen = std::mt19937(std::random_device {}());
    auto dist = std::uniform_real_distribution<float>(0.0f, 1.0f);
    return dist(gen);
}

// mirrored in frag.glsl
struct Chunk {
    Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

    void init()
    {
        glm::vec3 center = glm::vec3(CHUNK_SIZE) / 2.0f;

        for (uint32_t x = 0; x < CHUNK_SIZE; x++) {
            for (uint32_t y = 0; y < CHUNK_SIZE; y++) {
                for (uint32_t z = 0; z < CHUNK_SIZE; z++) {
                    glm::vec3 p = glm::vec3(x, y, z);

                    voxels[x][y][z] = {
                        .emission = glm::vec3(randf() < 0.01, randf() < 0.01, randf() < 0.01),
                        .diffuse = glm::vec3(0.5f) + 0.2f * glm::vec3(randf(), randf(), randf()),
                        .dbColor = { glm::vec3(), glm::vec3() },
                        .flags = glm::length(p - center) > 15.0 ? 1u : 0u,
                    };
                };
            }
        }
    }
};
