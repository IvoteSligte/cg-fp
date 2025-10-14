#version 430

in vec2 fragPos;
out vec4 fragColor;

// mirrored with main.cpp
struct Voxel {
    vec3 color;
    // bit 0 set indicates that the voxel exists
    uint flags;
};

// mirrored with main.cpp
// size of the voxel chunk in one dimension
const uint CHUNK_SIZE = 64;

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
    vec3 origin;
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
    vec3 invDirection = sign(ray.direction) / (abs(ray.direction) + 1e-5);

    vec3 position = ray.origin - CHUNK_MIN;
    vec3 step = sign(invDirection);
    vec3 delta = abs(invDirection);
    vec3 select = sign(invDirection) * 0.5 + 0.5;
    vec3 t = select * invDirection;
    
    while (true) {
        if (t.x < t.y) {
            if (t.x < t.z) {
                position.x += step.x;
                t.x += delta.x;
            } else {
                position.z += step.z;
                t.z += delta.z;
            }
        } else {
            if (t.y < t.z) {
                position.y += step.y;
                t.y += delta.y;
            } else {
                position.z += step.z;
                t.z += delta.z;
            }
        }

        // FIXME: snapping when moving because index gets snapped?
        ivec3 index = ivec3(position);
        if (any(lessThan(index, ivec3(0))) || any(greaterThan(index, ivec3(CHUNK_SIZE)))) {
            return RayCast(false, vec3(0.0), ivec3(0));
        }

        Voxel voxel = chunk.voxels[index.x][index.y][index.z];

        if (bitFlag(voxel.flags, 0)) {
            return RayCast(true, position + CHUNK_MIN, index);
        }
    }
}

// lowbias32 from https://nullprogram.com/blog/2018/07/31/
uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

uvec3 hash(uvec3 v) {
    return uvec3(hash(v.x), hash(v.y), hash(v.z));
}

uint hash(float x) {
    return hash(floatBitsToUint(x));
}

void main() {
    vec2 aspectRatio = vec2(1.0); // TODO:
    
    vec2 screenPos = (fragPos * 2.0 - 1.0) * aspectRatio;
    vec3 direction = rotation * normalize(vec3(screenPos.x, screenPos.y, -1.0));
    Ray ray = Ray(position, direction);

    // camera is outside chunk
    if (any(lessThan(ray.origin, CHUNK_MIN)) || any(greaterThan(ray.origin, CHUNK_MAX))) {
        // magenta (indicating error)
        fragColor = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }
    RayCast rayCast = rayCast(ray);
    vec3 color = vec3(0.0);

    if (rayCast.hit) {
        ivec3 index = rayCast.voxelIndex;
        color = chunk.voxels[index.x][index.y][index.z].color;
    } else {
        color = vec3(0.2, 0.2, 0.7);
    }

    color += 0.3 * (direction * 0.5 + 0.5);
    fragColor = vec4(color, 1.0);
}
