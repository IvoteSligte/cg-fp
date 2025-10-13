#version 430

in vec2 fragPos;
out vec4 fragColor;

// mirrored with main.cpp
struct Voxel {
    // bit 0 set indicates that the voxel exists
    uint flags;
};

// mirrored with main.cpp
// size of the voxel chunk in one dimension
const uint CHUNK_SIZE = 32;

// NOTE: names used for sync with CPU, do not change
layout(location = 0) uniform vec3 position;
layout(location = 1) uniform mat3 rotation;

layout(std430, binding = 0) readonly buffer Chunk {
    Voxel[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] voxels;
} chunk;

bool bitFlag(uint flags, uint index) {
    return (flags & (1u << index)) == 1u;
}

struct Ray {
    vec3 position;
    vec3 direction;
};

struct RayCast {
    bool hit;
    vec3 position;
    ivec3 voxelIndex;
};

const vec3 CHUNK_MIN = vec3(-float(CHUNK_SIZE) / 2.0);
const vec3 CHUNK_MAX = vec3(float(CHUNK_SIZE) / 2.0);

// Casts a ray, returning hit information.
// Assumes the ray falls within the chunk.
RayCast rayCast(Ray ray) {
    float stepLength = max(max(abs(ray.direction).x, abs(ray.direction).y), abs(ray.direction).z);
    vec3 step = ray.direction / stepLength;
    vec3 position = ray.position - CHUNK_MIN;
    int i = 0;
    
    while (i < 100) {
        i += 1;
        ivec3 index = ivec3(round(position));
        Voxel voxel = chunk.voxels[index.x][index.y][index.z];

        if (bitFlag(voxel.flags, 0)) {
            return RayCast(true, position + CHUNK_MIN, index);
        }
        if (any(lessThan(ray.position, vec3(0.0))) || any(greaterThan(ray.position, vec3(CHUNK_SIZE)))) {
            return RayCast(false, vec3(0.0), ivec3(0));
        }
    }
}

void main() {
    vec2 aspectRatio = vec2(1.0); // TODO:
    
    vec2 screenPos = (fragPos * 2.0 - 1.0) * aspectRatio;
    vec3 direction = rotation * normalize(vec3(screenPos.x, 1.0, screenPos.y));
    Ray ray = Ray(position, direction);

    // camera is outside chunk
    if (any(lessThan(ray.position, CHUNK_MIN)) || any(greaterThan(ray.position, CHUNK_MAX))) {
        // magenta (indicating error)
        fragColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
    RayCast rayCast = rayCast(ray);
    vec3 color = vec3(0.0);

    if (rayCast.hit) {
        color = vec3(1.0, 0.0, 0.0);
        // color = rayCast.position / CHUNK_SIZE * 0.5 + 0.5;
    } else {
        color = vec3(0.2, 0.2, 0.7);
    }
    fragColor = vec4(color, 1.0);
}
