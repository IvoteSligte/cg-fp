// compute shader
#version 430

// mirrored with main.cpp
// NOTE: order is important due to alignment
struct Voxel {
    vec3 emission;
    uint _padding0;
    vec3 diffuse;
    // bit 0 set indicates that the voxel exists
    uint flags;
};

// mirrored with main.cpp
// size of the voxel chunk in one dimension
const uint CHUNK_SIZE = 32;

layout(std430, binding = 0) readonly buffer Chunk {
    Voxel[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] voxels;
} chunk;

void main() {
    uvec3 id = gl_GlobalInvocationID;

    // TODO:
}
