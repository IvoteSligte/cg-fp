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

void main() {
    uvec3 index = gl_GlobalInvocationID;

    chunk.voxels[index.x][index.y][index.z].color = uvec3(1);

    // TODO:
}
