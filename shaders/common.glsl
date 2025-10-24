// NOTE: names used for sync with CPU, do not change
layout(location = 0) uniform uint dbColorReadIdx;

// mirrored with main.cpp
struct Voxel {
    vec3 emission;
    vec3 diffuse;
    // Double-buffered color.
    // The read index is indicated by dbColorReadIdx.
    vec3 dbColor[2];
    // bit 0 set indicates that the voxel exists
    uint flags;
};

// mirrored with main.cpp
// size of the voxel chunk in one dimension
const uint CHUNK_SIZE = 32;

layout(std430, binding = 0) buffer Chunk {
    Voxel[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] voxels;
} chunk;

bool bitFlag(uint flags, uint index) {
    return (flags & (1u << index)) == 1u;
}

Voxel getVoxel(ivec3 index) {
    return chunk.voxels[index.x][index.y][index.z];
}

// returns true if VOXEL is solid
bool isSolid(Voxel voxel) {
    return bitFlag(voxel.flags, 0);
}

vec3 getColor(Voxel voxel) {
    return voxel.dbColor[dbColorReadIdx];
}

void setColor(ivec3 index, vec3 color) {
    uint writeIdx = 1 - dbColorReadIdx;
    chunk.voxels[index.x][index.y][index.z].dbColor[writeIdx] = color;
}

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct RayCast {
    bool hit;
    vec3 position;
    ivec3 voxelIndex;
    int steps;
};

bool isOutOfBounds(vec3 position) {
    return any(lessThan(position, vec3(0.0))) || any(greaterThanEqual(position, vec3(CHUNK_SIZE)));
}

bool isOutOfBounds(ivec3 position) {
    return any(lessThan(position, ivec3(0))) || any(greaterThanEqual(position, ivec3(CHUNK_SIZE)));
}

// Casts a ray, returning hit information.
// Assumes ray.position is within the chunk.
RayCast rayCast(Ray ray) {
    vec3 invDirection = sign(ray.direction) / (abs(ray.direction) + 1e-5);

    vec3 position = ray.origin;
    vec3 step = sign(invDirection);
    vec3 delta = abs(invDirection);
    vec3 select = sign(invDirection) * 0.5 + 0.5;
    vec3 t = (select - fract(position)) * invDirection;

    int i = 0;
    while (true) {
        // TEMP
        if (i > 10000) {
            return RayCast(false, vec3(0.0), ivec3(0), i);
        }
        i += 1;

        int n = 2;
        if (t.x < t.y) {
            if (t.x < t.z) n = 0;
        } else {
            if (t.y < t.z) n = 1;
        }
        position[n] += step[n];
        t[n] += delta[n];

        ivec3 index = ivec3(position);
        if (isOutOfBounds(index)) {
            return RayCast(false, vec3(0.0), ivec3(0), i);
        }
        if (isSolid(getVoxel(index))) {
            return RayCast(true, position, index, i);
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


