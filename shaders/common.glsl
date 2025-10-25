// NOTE: names used for sync with CPU, do not change
layout(location = 0) uniform uint dbColorReadIdx;

// mirrored with main.cpp
struct Voxel {
    vec3 emission;
    vec3 diffuse;
    // Double-buffered color per face.
    // The read index is indicated by dbColorReadIdx.
    vec3 dbFaceColor[6][2];
    // bit 0 set indicates that the voxel exists
    uint flags;
};

// mirrored with main.cpp
// size of the voxel chunk in one dimension
const uint CHUNK_SIZE = 32;

layout(std430, binding = 0) buffer Chunk {
    Voxel[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] voxels;
} chunk;

// const vec3 SKY_COLOR = vec3(0.2, 0.2, 0.7);
const vec3 SKY_COLOR = vec3(0.0);
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

vec3 getColor(Voxel voxel, uint face) {
    return voxel.dbFaceColor[face][dbColorReadIdx];
}

void setColor(ivec3 index, uint face, vec3 color) {
    uint writeIdx = 1 - dbColorReadIdx;
    chunk.voxels[index.x][index.y][index.z].dbFaceColor[face][writeIdx] = color;
}

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct RayCast {
    bool hit;
    vec3 position;
    ivec3 voxelIndex;
    uint face;
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

// Returns outDirection snapped to the nearest voxel face.
vec3 voxelNormal(vec3 outDirection) {
    vec3 dir = abs(outDirection);
    // normal is that of one of six faces, of three dimensions (d)
    int d = 2;
    if (dir.x > dir.y) {
        if (dir.x > dir.z) d = 0;
    } else {
        if (dir.y > dir.z) d = 1;
    }
    vec3 norm = vec3(0.0);
    norm[d] = sign(outDirection)[d];
    return norm;
}

// Returns an integer representing the face of a voxel based on the normal.
uint faceVoxelNormal(vec3 outDirection) {
    vec3 dir = abs(outDirection);
    // normal is that of one of six faces, of three dimensions (d)
    int d = 2;
    if (dir.x > dir.y) {
        if (dir.x > dir.z) d = 0;
    } else {
        if (dir.y > dir.z) d = 1;
    }
    return d * 2 + uint(outDirection[d] > 0);
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
    // dimension along which the last step was taken
    uint dim = 0;

    while (true) {
        // ivec3(position) truncates towards zero and is therefore not the same
        // for negative values
        ivec3 index = ivec3(floor(position));
        if (isOutOfBounds(index)) {
            return RayCast(false, vec3(0.0), ivec3(0), 0);
        }
        if (isSolid(getVoxel(index))) {
            // step[dim] < 0 instead of step[dim] > 0
            // since the normal is in the opposite direction of the last step
            uint face = dim * 2 + uint(step[dim] < 0);
            return RayCast(true, position, index, face);
        }

        dim = 2;
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

// murmurHash14 from https://gist.github.com/mpottinger/54d99732d4831d8137d178b4a6007d1a
uint hash(uvec4 src) {
    const uint M = 0x5bd1e995u;
    uint h = 1190494759u;
    src *= M; src ^= src>>24u; src *= M;
    h *= M; h ^= src.x; h *= M; h ^= src.y; h *= M; h ^= src.z; h *= M; h ^= src.w;
    h ^= h>>13u; h *= M; h ^= h>>15u;
    return h;
}
