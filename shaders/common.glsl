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

const vec3 SKY_COLOR = vec3(0.2, 0.2, 0.7);
const vec3 ERROR_COLOR = vec3(1.0, 0.0, 1.0); // magenta

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

// 1.0 / vector, mapping zero-dimensions to 1e30
vec3 invert(vec3 v) {
    return vec3(
        v.x == 0 ? 1e30 : 1.0 / v.x,
        v.y == 0 ? 1e30 : 1.0 / v.y,
        v.z == 0 ? 1e30 : 1.0 / v.z
    );
}

// Casts a ray, returning hit information.
// Assumes ray.position is within the chunk.
RayCast rayCast(Ray ray) {
    vec3 invDirection = invert(ray.direction);

    vec3 position = floor(ray.origin);
    vec3 step = sign(ray.direction);
    vec3 delta = abs(invDirection);
    // relative boundary of the next voxel
    vec3 boundary = max(step, vec3(0.0));
    vec3 t = (boundary - fract(ray.origin)) * invDirection;

    int i = 0;
    while (true) {
        // ivec3(position) truncates towards zero and is therefore not the same
        // for negative values
        ivec3 index = ivec3(floor(position));
        if (isOutOfBounds(index)) {
            return RayCast(false, vec3(0.0), ivec3(0), i);
        }
        if (isSolid(getVoxel(index))) {
            return RayCast(true, position, index, i);
        }
        i += 1;

        int dim = 2;
        if (t.x < t.y) {
            if (t.x < t.z) dim = 0;
        } else {
            if (t.y < t.z) dim = 1;
        }
        position[dim] += step[dim];
        t[dim] += delta[dim];
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

// Random float in the range [0, 1]
float random(uint seed) {
    return hash(seed) * (1.0 / 4294967296.0);
}

const float PI = 3.14159265359;

// FIXME: direction bias
vec3 randomDirection(uint seed) {
    float theta = random(seed * 2868849829u + 3875816143u) * (2.0 * PI);
    float z = random(seed * 4156532671u + 2891336453u) * 2.0 - 1.0;
    // radius of the circle on which (x, y) lies
    float r = sqrt(1.0 - z * z);
    return vec3(r * cos(theta), r * sin(theta), z);
}

