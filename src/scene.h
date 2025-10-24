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

inline Voxel invertedSphereScene(glm::uvec3 point)
{
    glm::vec3 center = glm::vec3(CHUNK_SIZE) / 2.0f;
    float radius = (float)CHUNK_SIZE / 2.0 - 1.0;
    return Voxel {
        .emission = glm::vec3(randf() < 0.01, randf() < 0.01, randf() < 0.01),
        .diffuse = glm::vec3(0.5f) + 0.2f * glm::vec3(randf(), randf(), randf()),
        .dbColor = {},
        .flags = glm::length(glm::vec3(point) - center) > radius ? 1u : 0u,
    };
}

inline Voxel simpleScene(glm::uvec3 point)
{
    const Voxel WALL = Voxel {
        .emission = glm::vec3(),
        .diffuse = glm::vec3(1.0),
        .dbColor = {},
        .flags = 1u,
    };
    const Voxel AIR = Voxel {
        .flags = 0u,
    };
    const Voxel LIGHT = Voxel {
        .emission = glm::vec3(1.0, 0.9, 0.7),
        .diffuse = glm::vec3(0.0),
        .dbColor = {},
        .flags = 1u,
    };

    glm::vec3 lightPosition = glm::vec3(CHUNK_SIZE) / 2.0f + glm::vec3(0.0f, 4.0f, 0.0f);
    float lightRadius = CHUNK_SIZE / 8.0f;
    uint max = CHUNK_SIZE - 1;

    if (point.x == 0 || point.y == 0 || point.z == 0) {
        return WALL;
    }
    if (point.x == max || point.y == max || point.z == max) {
        return WALL;
    }
    if (glm::distance(glm::vec3(point), lightPosition) < lightRadius) {
        return LIGHT;
    }
    return AIR;
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
                    voxels[x][y][z] = invertedSphereScene(glm::uvec3(x, y, z));
                };
            }
        }
    }
};
