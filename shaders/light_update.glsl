// compute shader
#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

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

layout(location = 0) uniform uint dbColorReadIdx;

Voxel getVoxel(ivec3 index) {
    return chunk.voxels[index.x][index.y][index.z];
}

vec3 getColor(Voxel voxel) {
    return voxel.dbColor[dbColorReadIdx];
}

void setColor(ivec3 index, vec3 color) {
    uint writeIdx = 1 - dbColorReadIdx;
    chunk.voxels[index.x][index.y][index.z].dbColor[writeIdx] = color;
}

void main() {
    ivec3 index = ivec3(gl_GlobalInvocationID);
    vec3 emission = getVoxel(index).emission;
    vec3 color = getColor(getVoxel(index));
    color = max(color, emission);

    for (int x = -1; x <= 1; x += 1) {
        for (int y = -1; y <= 1; y += 1) {
            for (int z = -1; z <= 1; z += 1) {
                vec3 nbColor = getColor(getVoxel(ivec3(mod(ivec3(index) + ivec3(1), ivec3(CHUNK_SIZE)))));
                color = mix(color, nbColor, 0.0001);
            }
        }
    }

    setColor(index, color);
}
