// compute shader
#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

// mirrored with main.cpp
// NOTE: order is important due to alignment
struct Voxel {
    vec3 emission;
    uint _padding0;
    vec3 diffuse;
    uint _padding1;
    uvec3 color;
    // bit 0 set indicates that the voxel exists
    uint flags;
};

// mirrored with main.cpp
// size of the voxel chunk in one dimension
const uint CHUNK_SIZE = 32;

layout(std430, binding = 0) buffer Chunk {
    Voxel[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] voxels;
} chunk;

int offset(uint a, int b) {
    return int(mod(a + b, CHUNK_SIZE));
}

void main() {
    uvec3 index = gl_GlobalInvocationID;
    vec3 emission = chunk.voxels[index.x][index.y][index.z].emission;
    vec3 color = uintBitsToFloat(chunk.voxels[index.x][index.y][index.z].color);
    color = max(color, emission);

    for (int x = -1; x <= 1; x += 1) {
        for (int y = -1; y <= 1; y += 1) {
            for (int z = -1; z <= 1; z += 1) {
                vec3 nbColor = uintBitsToFloat(chunk.voxels[offset(index.x, 1)][offset(index.y, 1)][offset(index.z, 1)].color);
                color = mix(color, nbColor, 0.0001);
            }
        }
    }

    uvec3 uColor = floatBitsToUint(color);
    for (int i = 0; i < 3; i++) {
        atomicExchange(chunk.voxels[index.x][index.y][index.z].color[i], uColor[i]);
    }

    // TODO:
}
