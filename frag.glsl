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

layout(std430, binding = 0) readonly buffer Chunk {
    Voxel[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] voxels;
};

bool bitFlag(uint flags, uint index) {
    return (flags & (1u << index)) == 1u;
}

void main() {
    uvec2 index = uvec2(fragPos * CHUNK_SIZE);
    vec3 color = vec3(0.0);

    Voxel voxel = voxels[0][index.x][index.y];
    if (bitFlag(voxel.flags, 0)) {
        color = vec3(1.0);
    }

    fragColor = vec4(color, 1.0);
}
